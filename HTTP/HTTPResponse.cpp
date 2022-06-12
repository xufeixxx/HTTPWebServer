#include"HTTPResponse.hpp"
/*本程序没有考虑400错误*/
const unordered_map<int,string> HTTPResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const unordered_map<int, string> HTTPResponse::CODE_FILE = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HTTPResponse::HTTPResponse(){
    code_ = -1;
    isKeepAlive_ = false;
    path_ = "";
    srcDir_ = "";
    mmFile_ = nullptr;
    memset(&mmFileStat_,0,sizeof(mmFileStat_));
}

void HTTPResponse::UnmapFile(){
    if(mmFile_){
        munmap(mmFile_,mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}

HTTPResponse::~HTTPResponse(){
    UnmapFile();
}

void HTTPResponse::Init(const string& srcDir, const string& path, bool isKeepAlive, int code){
    assert(srcDir != "");
    if(mmFile_) { UnmapFile(); }
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    //cout <<"HTTPResponse初始化的时候path_ = "<<path_<<endl;
    srcDir_ = srcDir;
    mmFile_ = nullptr; 
    memset(&mmFileStat_,0,sizeof(mmFileStat_));
}

void* HTTPResponse::File(){
    return mmFile_;
}

size_t HTTPResponse::FileLen() const{
    return mmFileStat_.st_size;
}

int HTTPResponse::Code() const{
    return code_;
}

void HTTPResponse::AddStateLine_(Buffer& buff){
    buff.Append("HTTP/1.1 " + to_string(code_) + " " + CODE_STATUS.find(code_)->second + "\r\n");
}

void HTTPResponse::AddHeader_(Buffer& buff){
    buff.Append("Connection: ");
    if(isKeepAlive_) {
        buff.Append("keep-alive\r\n");
    } else{
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: text/html\r\n");
}

void HTTPResponse::AddContent_(Buffer& buff){
    if(code_ != 200){
        path_ = CODE_FILE.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    LOG_DEBUG("file path %s\n", (srcDir_ + path_).data());
    mmFile_ = mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    close(srcFd);//关闭内存映射对应的文件，不会影响映射区
    /*加两个\r\n的原因是：第二个\r\n是首部和实体间的空行*/
    buff.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

void HTTPResponse::MakeResponse(Buffer& buff){
    //cout << "srcDir_ + path_ = " << srcDir_ + path_ <<endl;
    if(stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        //cout << "srcDir_ + path_ = " << srcDir_ + path_ <<endl;
        code_ = 404;
    }
    else if(!(mmFileStat_.st_mode & S_IROTH)) {//S_IROTH其他组读权限
        code_ = 403;
    }
    else{ 
        code_ = 200; 
        //cout << srcDir_ + path_ <<endl;
    }
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}
