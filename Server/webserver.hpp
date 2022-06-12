#ifndef WEBSERVER_H
#define WEBSERVER_H

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unordered_map>
#include<functional>

#include"../HTTP/HTTPConn.hpp"
#include"epoller.hpp"
#include"../Log/log.hpp"
#include"../ThreadPool/threadpool.hpp"
#include"../Timer/heaptimer.hpp"

using namespace std;

class WebServer{
public:

    WebServer();
    ~WebServer();

    void Start();
private:
    bool InitSocket_(); 
    void AddClient_(int fd, sockaddr_in addr);

    void DealListen_();
    void DealWrite_(HTTPConn* client);
    void DealRead_(HTTPConn* client);

    void SendError_(int fd, const char*info);
    void CloseConn_(HTTPConn* client);
    void ExtentTime_(HTTPConn* client);

    void OnRead_(HTTPConn* client);//线程读函数，需要放入请求队列中
    void OnWrite_(HTTPConn* client);//线程写函数，需要放入请求队列中
    void OnProcess(HTTPConn* client);

    static const int MAX_FD = 65536;
    static int SetFdNonblock(int fd);

    int port_;
    bool isClose_;
    int listenFd_;
    
    uint32_t listenEvent_;
    uint32_t connEvent_;

    unique_ptr<HeapTimer> timer_;
    unique_ptr<ThreadPool<void()>> threadpool_;
    unique_ptr<Epoller> epoller_;
    unordered_map<int, HTTPConn> users_;

    static const unsigned int live_second_ = 20;//所有socket设置的超时时间都是当前时间加上live_second_
    static const unsigned int check_second = 10;//每过10秒调用一次定时器的tick()函数
};

#endif