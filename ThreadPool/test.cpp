//线程池测试程序
//考虑的函数类型为function<void()> 
#include"threadpool.hpp"
#include"iostream"

void read_process(){
    std::cout << "read_process" <<std::endl;
}

void write_process(){
    std::cout << "write_process" <<std::endl;
}

int main(){
    //主线程
    ThreadPool<void()> tp;

    for(int i = 0; i < 10; i++){
        if(i % 2 == 0){
            tp.addTask(read_process);
        }else{
            tp.addTask(write_process);
        }
    }

    while(1);
    return 0;
}