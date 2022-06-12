//日志系统的测试文件
//开启五个线程不断地写入信息
#include<unistd.h>
#include<iostream>
#include"Log/log.hpp"

locker mutex;

void* thread_process(void* arg){
    while(1){
        //mutex.lock();
        //std::cout<<pthread_self()<<"\n";
        LOG_INFO("writed by thread #%ld\n",pthread_self());
        //sleep(2);
        //mutex.unlock();
    }
}

int main(){

    Log::getInstance()->init(string("/home/xufei/文档/Linux Project/WebServer_new/LogFile/"),10000);

    pthread_t tids[5];

    for(int i = 0; i < 5; i++){
        pthread_create(&tids[i],nullptr,thread_process,nullptr);
    }

    for(int i = 0; i < 5; i++){
        pthread_detach(tids[i]);
    }

    while(1);

    return 0;

}
