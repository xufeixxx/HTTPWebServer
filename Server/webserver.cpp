#include"webserver.hpp"

WebServer::WebServer():port_(9190),isClose_(false),timer_(new HeapTimer()),
threadpool_(new ThreadPool<void()>()),epoller_(new Epoller()){

    SQLConnPool::getInstance()->init("192.168.16.134",3306,"root","root","xfdb",16);
    listenEvent_ = EPOLLRDHUP | EPOLLET;
    connEvent_ = EPOLLRDHUP | EPOLLONESHOT | EPOLLET;

    if(!InitSocket_()){
        isClose_ = false;
    }

    Log::getInstance()->init(string("/home/xufei/文档/Linux Project/WebServer_new/LogFile/"),50000);
    if(isClose_) { LOG_ERROR("========== Server init error!==========\n"); }
    else {
        LOG_INFO("========== Server init ==========\n");
        LOG_INFO("Port:%d\n", port_);
        LOG_INFO("Listen Mode: ET, OpenConn Mode: ET\n");
        LOG_INFO("SqlConnPool num: 16, ThreadPool num: 8\n");
    }


}

bool WebServer::InitSocket_(){
    int ret;
    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    listenFd_ = socket(AF_INET,SOCK_STREAM,0);
    if(listenFd_ < 0){
        LOG_ERROR("Create socket error!\n");
        return false;
    }

    /*端口复用*/
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !\n");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!\n", port_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!\n", port_);
        close(listenFd_);
        return false;
    }

    bool result = epoller_->AddFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(!result) {
        LOG_ERROR("Add listen error!\n");
        close(listenFd_);
        return false;
    }
    SetFdNonblock(listenFd_);
    LOG_INFO("Server port:%d\n", port_);
    return true;

}


int WebServer::SetFdNonblock(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

WebServer::~WebServer(){
    close(listenFd_);
    isClose_ = true;
    SQLConnPool::getInstance()->destoryPool();
}

void WebServer::DealListen_(){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    while(1){
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if(fd <= 0) { return;}
        else if(HTTPConn::userCount >= MAX_FD) {
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!\n");
            return;
        }
        AddClient_(fd, addr);
    }
}

void WebServer::SendError_(int fd, const char*info){
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!\n", fd);
    }
    close(fd);
}

void WebServer::AddClient_(int fd, sockaddr_in addr){
    assert(fd > 0);
    users_[fd].init(fd, addr);
    function<void()> cb_func(bind(&WebServer::CloseConn_,this,&users_[fd]));
    TimerNode tn(fd,static_cast<time_t>(time(nullptr)+live_second_),cb_func);
    timer_->push(tn);
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!\n", users_[fd].getFD());
}

void WebServer::CloseConn_(HTTPConn* client){
    assert(client);
    LOG_INFO("Client[%d] quit!\n", client->getFD());
    epoller_->DelFd(client->getFD());
    client->Close();
}

void WebServer::DealWrite_(HTTPConn* client){
    assert(client);
    ExtentTime_(client);
    threadpool_->addTask(bind(&WebServer::OnWrite_,this,client));
}

void WebServer::DealRead_(HTTPConn* client){
    assert(client);
    ExtentTime_(client);
    threadpool_->addTask(bind(&WebServer::OnRead_,this,client));
}

void WebServer::ExtentTime_(HTTPConn* client){
    heap_index hindex = timer_->map_index(client->getFD());
    timer_->addExpireTime(hindex,time(nullptr)+live_second_);
}

void WebServer::OnRead_(HTTPConn* client){
    assert(client);
    //cout << "当前线程为:" << pthread_self() <<"\n";
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite_(HTTPConn* client){
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        /* 传输完成 */
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            epoller_->ModFd(client->getFD(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}

void WebServer::OnProcess(HTTPConn* client){
    if(client->process()) {
        epoller_->ModFd(client->getFD(), connEvent_ | EPOLLOUT);
    } else {
        epoller_->ModFd(client->getFD(), connEvent_ | EPOLLIN);
    }
}

void WebServer::Start(){
    if(!isClose_) { LOG_INFO("========== Server start ==========\n"); }
    time_t cur = time(nullptr);
    while(!isClose_) {
        if(time(nullptr) - cur >= 10){
            timer_->tick();
            cur = time(nullptr);
        }
        int eventCnt = epoller_->Wait();
        for(int i = 0; i < eventCnt; i++) {
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if(fd == listenFd_) {
                DealListen_();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }
            else if(events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            else if(events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event\n");
            }
        }
    }
}

