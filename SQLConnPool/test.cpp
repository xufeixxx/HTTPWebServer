//数据库连接池测试程序
//创建一个容量为4的连接池，创建8个线程并发的对数据库进行操作

#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#include"sqlconnpool.hpp"
#include"sqlconnRAII.hpp"

/*void* thread_process(void*){
    while(1){
        MYSQL* conn = SQLConnPool::getInstance()->getConnection();
        char sql_order[256] = {0};
        snprintf(sql_order,256,"insert into thread_info(thread_id) values('%ld')",pthread_self());
        mysql_query(conn,sql_order);
        printf("线程%ld插入完成\n",pthread_self());
        SQLConnPool::getInstance()->releaseConnection(conn);
        sleep(2);
    }
}*/


int main(){

    /*    SQLConnPool::getInstance()->init("192.168.16.134",3306,"root","root","qgydb",4);

    for(int i = 0; i < 8; i++){
        pthread_t tid;
        pthread_create(&tid,nullptr,thread_process,nullptr);
        pthread_detach(tid);
    }*/
    SQLConnPool::getInstance()->init("192.168.16.134",3306,"root","root","xfdb",4);

    string username = "root";
    string password = "r1oot";

    MYSQL* conn = SQLConnPool::getInstance()->getConnection();
    assert(conn);
    char sql_order[256] = {0};
    snprintf(sql_order,256,"SELECT * FROM user WHERE username = '%s' AND password = '%s'",username.c_str(),password.c_str());
    if(!mysql_query(conn,sql_order)){
        MYSQL_RES* result = mysql_store_result(conn);
        printf("结果集中行的数量是%d\n",static_cast<int>(mysql_num_rows(result)));
        printf("结果集中列的数量是%d\n",mysql_num_fields(result));
    }

    
    //printf("查询到的数量是：%d\n",static_cast<int>(mysql_affected_rows(conn)));

    while(1);



    return 0;
}