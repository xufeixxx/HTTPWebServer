#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include<fcntl.h>
#include<unistd.h>
#include<unordered_map>
#include<string>
#include<sys/mman.h>
//使用struct stat和stat()函数需要的头文件，以此来获取文件相应的属性
#include<sys/stat.h>
#include<iostream>

#include"../Buffer/buffer.hpp"
#include"../Log/log.hpp"

using namespace std;

class HTTPResponse{
public:
    HTTPResponse();
    ~HTTPResponse();

    void UnmapFile();
    void Init(const string& srcDir, const string& path, bool isKeepAlive = false, int code = -1);
    void* File();
    size_t FileLen() const;
    int Code() const;

    void MakeResponse(Buffer& buff);

private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    int code_;//响应报文的状态码
    bool isKeepAlive_;

    string path_;
    string srcDir_;

    /*文件的内存映射*/
    void* mmFile_;//指向文件映射的内存
    struct stat mmFileStat_;//存储文件相关信息

    static const unordered_map<int, string> CODE_STATUS;
    static const unordered_map<int, string> CODE_FILE;
};





#endif