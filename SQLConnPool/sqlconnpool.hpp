#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include<queue>
#include<list>
#include<mysql/mysql.h>
#include<string>
#include<assert.h>
#include"../Log/log.hpp"
#include"../lock/locker.hpp"

using namespace std;

class SQLConnPool{
public:

    static SQLConnPool* getInstance();
    void init(string host,int port,string user,string passwd,string dbname,unsigned int maxconn = 10);
    MYSQL* getConnection();
    void releaseConnection(MYSQL* conn);
    void destoryPool();

private:
    SQLConnPool(){}
    ~SQLConnPool();
private:

    queue<MYSQL*,list<MYSQL*>> conn_pool;
    unsigned int MAX_CONN;
    locker mutex;
    sem remain;

};

#endif