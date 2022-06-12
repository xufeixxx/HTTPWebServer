#include"HTTPConn.hpp"

const string HTTPConn::srcDir = "/home/xufei/文档/Linux Project/WebServer_new/HTML";
atomic<int> HTTPConn::userCount = 0;

HTTPConn::HTTPConn(){
    fd_ = -1;
    memset(&addr_,0,sizeof(addr_));
    isClose_ = false;
}

void HTTPConn::init(int sockFD, const sockaddr_in& addr){
    userCount++;
    addr_ = addr;
    fd_ = sockFD;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
}

void HTTPConn::Close(){
    response_.UnmapFile();
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        close(fd_);
    }
}

HTTPConn::~HTTPConn(){
    Close();
}

int HTTPConn::getFD() const{
    return fd_;
}

int HTTPConn::getPort() const{
    return addr_.sin_port;
}

const char* HTTPConn::getIP() const{
    return inet_ntoa(addr_.sin_addr);
}

sockaddr_in HTTPConn::getAddr() const{
    return addr_;
}

ssize_t HTTPConn::read(int* saveErrno){//et模式下的读操作
    ssize_t len = -1;
    while(1){
        len = readBuff_.ReadFd(fd_,saveErrno);
        if(len < 0)break;
    }
    // cout<<"函数为HTTPConn::read\n";
    // readBuff_.getContent();
    return len;
}

bool HTTPConn::process(){
    request_.init();
    if(readBuff_.ReadableBytes() <= 0){
        return false;
    }
    request_.parse(readBuff_);
    //cout << "响应文件为："<<request_.path()<< "线程为:"<< pthread_self() <<"\n";
    response_.Init(srcDir,request_.path(),request_.IsKeepAlive(),200);
    //cout <<"HTTPResponse初始化的时候path_ = "<<request_.path()<<endl;

    response_.MakeResponse(writeBuff_);

    /* 响应头 */
    iov_[0].iov_base = const_cast<char*>(writeBuff_.Peek());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    /* 文件 */
    if(response_.FileLen() > 0  && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }

    return true;
}


ssize_t HTTPConn::write(int* saveErrno){
    ssize_t len = -1;
    while(1){
        len = writev(fd_,iov_,iovCnt_);
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len == 0 && iov_[1].iov_len == 0){//传输完成
            break;
        }else if(static_cast<size_t>(len) > iov_[0].iov_len){
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }else{
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuff_.Retrieve(len);
        }
    }
    return len;
}