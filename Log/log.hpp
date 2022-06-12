#ifndef LOG_H
#define LOG_H

#include<iostream>
#include<pthread.h>
#include<list>
#include<queue>
#include<string>

#include"../lock/locker.hpp"

using namespace std;

class Log{
public:
    static Log* getInstance();
    void init(const string& filepath, int max_lines);
    void write_log(int level,const char* format,...);
private:
    Log(int queue_size = 8);
    ~Log(){}
    static void* thread_process(void* arg);//线程处理函数，类中必须是静态函数，可以调用非静态成员变量，需要借助一非静态函数
    void read_process();
private:
    string m_filename;//日志文件名
    string m_filepath;//日志文件存放的位置
    FILE* m_pf;

    int m_max_lines; // 单个日志文件的最大行数
    long long m_total_lines;// 某一天的所有日志文件当前的总行数

    queue<string,list<string>> m_queue;
    int m_queue_size;//构造函数初始化

    locker m_mutex;
    sem m_sem_full;//构造函数初始化
    sem m_sem_empty;//构造函数初始化

    pthread_t m_tid;

    int m_year;
    int m_mon;
    int m_day;//标记当前是哪天
};

#define LOG_ERROR(format, ...) Log::getInstance()->write_log(1, format, ##__VA_ARGS__);  
#define LOG_INFO(format, ...) Log::getInstance()->write_log(2, format, ##__VA_ARGS__);  
#define LOG_DEBUG(format, ...) Log::getInstance()->write_log(3, format, ##__VA_ARGS__);  
#define LOG_WARN(format, ...) Log::getInstance()->write_log(4, format, ##__VA_ARGS__);  

#endif