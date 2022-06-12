#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H

#include"sqlconnpool.hpp"

class SQLConnRAII{
public:
    SQLConnRAII(MYSQL** conn,SQLConnPool* pool){
        assert(pool);
        pool_ = pool;
        *conn = pool_->getConnection();
        conn_ = *conn;
    }
    ~SQLConnRAII(){
        if(conn_){
            pool_->releaseConnection(conn_);
        }
    }
private:
    MYSQL* conn_;
    SQLConnPool* pool_;
};

#endif