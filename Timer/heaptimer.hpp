#pragma once
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include<vector>
#include<unordered_map>
#include<time.h>
#include<algorithm>
#include<assert.h>
#include<functional>

#include"../HTTP/HTTPConn.hpp"

using namespace std;

using heap_index = unsigned int;//标记堆数组索引

static const size_t HEAP_INIT_SIZE = 64;

struct TimerNode {
    int sockfd;
    time_t expire_time;
    function<void()> cb_func_;//在节点中的socket超时的时候调用，主要是关闭连接删除epoll中注册的事件
    TimerNode(int sockfd_, time_t expire_time_, function<void()> cb_func) :sockfd(sockfd_), expire_time(expire_time_),
        cb_func_(cb_func) {}
};

class HeapTimer {//小根堆按照失效时间构建小根堆
public:
    HeapTimer() { heap_.reserve(HEAP_INIT_SIZE); }
    ~HeapTimer() {}

    TimerNode& top();
    void push(TimerNode);
    void pop();
    bool empty() { return heap_.size() == 0; }
    bool addExpireTime(heap_index, time_t);
    heap_index map_index(int);//映射函数，根据sockfd得到所在heap_的索引
    void tick();

private:

    heap_index upshift(heap_index);
    heap_index downshift(heap_index);
    void SwapNode(heap_index, heap_index);

private:
    vector<TimerNode> heap_;
    unordered_map<int, heap_index> map_;//记录socketfd对应的vector中的索引
};

#endif