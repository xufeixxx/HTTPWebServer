#ifndef HTTPCONN_H
#define HTTPCONN_H

#include<arpa/inet.h>
#include<sys/uio.h>
#include<error.h>
#include<string>
#include<stdlib.h>
#include<iostream>

#include"../Buffer/buffer.hpp"
#include"HTTPRequest.hpp"
#include"HTTPResponse.hpp"

using namespace std;

class HTTPConn{
public:

    HTTPConn();
    ~HTTPConn();

    void init(int sockFD, const sockaddr_in& addr);
    void Close();

    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    bool process();

    int getFD() const;
    int getPort() const;
    const char* getIP() const;
    sockaddr_in getAddr() const;

    static const string srcDir;//映射文件所在的根目录
    static atomic<int> userCount;//用户数量，对应着HTTPConn的数量

    int ToWriteBytes() { 
        return iov_[0].iov_len + iov_[1].iov_len; 
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

private:

    int fd_;//socket的文件描述符
    sockaddr_in addr_;//socket的地址

    bool isClose_;

    Buffer readBuff_;
    Buffer writeBuff_;

    HTTPRequest request_;
    HTTPResponse response_;

    int iovCnt_;
    iovec iov_[2];
};

#endif