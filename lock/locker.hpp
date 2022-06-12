#ifndef LOCKER_H
#define LOCKER_H
#include<pthread.h>
#include<exception>
#include<semaphore.h>
class locker{
public:
    locker(){
        if(pthread_mutex_init(&_mutex,nullptr) != 0){
            throw std::exception();
        }
    }
    ~locker(){
        pthread_mutex_destroy(&_mutex);
    }
    bool lock(){
        return pthread_mutex_lock(&_mutex) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&_mutex) == 0;
    }
    pthread_mutex_t* get(){
        return &_mutex;
    }
private:
    pthread_mutex_t _mutex;
};
class cond{
public:
    cond(){
        if(pthread_cond_init(&_cond,nullptr) != 0){
            throw std::exception();
        }
    }
    ~cond(){
        pthread_cond_destroy(&_cond);
    }
    bool wait(pthread_mutex_t* pmutex){
        return pthread_cond_wait(&_cond,pmutex) == 0;
    }
    bool timedwait(pthread_mutex_t* pmutex,struct timespec time){
        return pthread_cond_timedwait(&_cond,pmutex,&time) == 0;
    }
    bool signal(){
        return pthread_cond_signal(&_cond) == 0;
    }
    bool broadcast(){
        return pthread_cond_broadcast(&_cond) == 0;
    }

private:
    pthread_cond_t _cond;
};
class sem{
public:
    sem(int num = 0){
        if(sem_init(&_sem,0,num) != 0){
            throw std::exception();
        }
    }
    ~sem(){
        sem_destroy(&_sem);
    }
    bool wait(){
        return sem_wait(&_sem) == 0;
    }
    bool post(){
        return sem_post(&_sem) == 0;
    }
private:
    sem_t _sem;
};
#endif