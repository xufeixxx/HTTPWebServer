
#include"sqlconnpool.hpp"

SQLConnPool* SQLConnPool::getInstance(){
    static SQLConnPool cp;
    return &cp;
}

void SQLConnPool::init(string host,int port,string user,string passwd,string dbname,unsigned int maxconn){
    assert(maxconn > 0);
    for(int i = 0; i < maxconn; i++){
        MYSQL* conn = nullptr;
        conn = mysql_init(conn);
        if (!conn) {
            LOG_ERROR("%s","MySQL init error!\n");
            assert(conn);
        }
        conn = mysql_real_connect(conn, host.c_str(), user.c_str(), passwd.c_str(), dbname.c_str(), port, nullptr, 0);
        if (!conn) {
            LOG_ERROR("%s","MySQL init error!\n");
        }
        conn_pool.push(conn);
    }
    MAX_CONN = maxconn;
    remain = sem(MAX_CONN);
}

MYSQL* SQLConnPool::getConnection(){
    remain.wait();
    mutex.lock();
    MYSQL* conn = conn_pool.front();
    conn_pool.pop();
    mutex.unlock();
    return conn;
}

void SQLConnPool::releaseConnection(MYSQL* conn){
    assert(conn);
    mutex.lock();
    conn_pool.push(conn);
    mutex.unlock();
    remain.post();
}

void SQLConnPool::destoryPool(){
    mutex.lock();
    while(!conn_pool.empty()) {
        MYSQL* conn = conn_pool.front();
        mysql_close(conn);
        conn_pool.pop();
    }
    mysql_library_end(); 
    mutex.unlock();
}

SQLConnPool::~SQLConnPool(){
    destoryPool();
}