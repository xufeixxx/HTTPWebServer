#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<pthread.h>
#include<queue>
#include<list>
#include<functional>
#include<assert.h>
#include"../lock/locker.hpp"

using namespace std;

template<typename func_type>
class ThreadPool{
public:

    ThreadPool(int threadCount = 8):sem_count_(0),isClosed(false){
        assert(threadCount > 0);
        for(int i = 0; i < threadCount; i++){
            pthread_t tid;
            pthread_create(&tid,nullptr,thread_process,this);
            pthread_detach(tid);
        }
    }

    ~ThreadPool(){
        isClosed = true;
    }

    void addTask(function<func_type>);

private:
    static void* thread_process(void* arg);
    void run();
private:

    locker mutex_;
    sem sem_count_;
    queue<function<func_type>,list<function<func_type>>> tasks;
    bool isClosed;

};
template<typename func_type>
void ThreadPool<func_type>::run(){
    while(true){
        if(isClosed)break;//线程池对象被销毁时isClosed设置为true，停止线程执行函数，线程结束，也就是说线程是手动结束的
        sem_count_.wait();
        mutex_.lock();
        if(tasks.empty()){
            mutex_.unlock();
            continue;
        }
        auto task = tasks.front();
        tasks.pop();
        mutex_.unlock();
        task();//运行队列中的function
    }
}
template<typename func_type>
void* ThreadPool<func_type>::thread_process(void* arg){
    ThreadPool* ptp = (ThreadPool*)arg;
    ptp->run();
    return ptp;
}
template<typename func_type>
void ThreadPool<func_type>::addTask(function<func_type> func){
    mutex_.lock();
    tasks.push(func);
    mutex_.unlock();
    sem_count_.post();
}



#endif