#include"log.hpp"
#include<ctime>
#include<stdarg.h>

Log::Log(int queue_size):m_sem_full(0),m_sem_empty(queue_size){}

Log* Log::getInstance(){
    static Log log;
    return &log;
}

void* Log::thread_process(void* arg){
    Log::getInstance()->read_process();
    return nullptr;
}

void Log::write_log(int level,const char* format,...){
    m_sem_empty.wait();
    time_t tt = time(nullptr);
    tm* ptm = localtime(&tt);

    va_list va;
    va_start(va,format);
    char buf[1024] = {0};
    vsnprintf(buf,1023,format,va);
    string logbody(buf);
    va_end(va);

    string type;
    switch(level){
    case 1:
        type = "ERROR";
        break;
    case 2:
        type = "INFO";
        break;
    case 3:
        type = "DEBUG";
        break;
    case 4:
        type = "WARN";
        break;
    default:
        break;
    }

    string loghead = to_string(1900+ptm->tm_year)+"-"+to_string(ptm->tm_mon+1)+"-"+to_string(ptm->tm_mday)+" "+
    to_string(ptm->tm_hour)+":"+to_string(ptm->tm_min)+":"+to_string(ptm->tm_sec)+"["+type+"]"+": ";

    string log = loghead + logbody;
    m_mutex.lock();
    m_queue.push(log);
    m_mutex.unlock();
    m_sem_full.post();
}

void Log::read_process(){
    while(1){
        m_sem_full.wait();
        m_mutex.lock();
        string str = m_queue.front();
        m_queue.pop();

        time_t tt = time(nullptr);
        tm* ptm = localtime(&tt);

        if(ptm->tm_year+1900 == m_year && ptm->tm_mon+1 == m_mon && ptm->tm_mday == m_day){
            if(m_total_lines % m_max_lines == 0){
                m_filename = to_string(m_year)+"_"+to_string(m_mon)+"_"+to_string(m_day)+
                string("_WebServerLog_")+to_string(m_total_lines/m_max_lines);
                m_pf = fopen((m_filepath+m_filename).c_str(),"a");
            }
        }else{
            m_day = ptm->tm_mday;
            m_year = ptm->tm_year+1900;
            m_mon = ptm->tm_mon+1;
            m_total_lines = 0;
            m_filename = to_string(m_year)+"_"+to_string(m_mon)+"_"+to_string(m_day)+
            string("_WebServerLog_")+to_string(m_total_lines/m_max_lines);
            m_pf = fopen((m_filepath+m_filename).c_str(),"a");
        }
        fputs(str.c_str(),m_pf);
        fflush(m_pf);
        m_total_lines++;
        m_mutex.unlock();
        m_sem_empty.post();
    }
}

void Log::init(const string& filepath, int max_lines){
    time_t tt = time(nullptr);
    tm* ptm = localtime(&tt);

    m_filepath = filepath;

    m_filename = to_string(ptm->tm_year+1900)+"_"+to_string(ptm->tm_mon+1)+"_"+to_string(ptm->tm_mday)+
    string("_WebServerLog")+"_0";

    // string totalpath = m_filepath+m_filename;

    m_pf = fopen((m_filepath+m_filename).c_str(),"a");
    if(m_pf == nullptr){
        exit(-1);
    }

    pthread_create(&m_tid,nullptr,thread_process,nullptr);

    m_day = ptm->tm_mday;
    m_year = ptm->tm_year+1900;
    m_mon = ptm->tm_mon+1;

    m_max_lines = max_lines;

    m_total_lines = 0;
}