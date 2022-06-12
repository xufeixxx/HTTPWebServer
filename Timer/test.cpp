//小根堆测试程序
//没有使用cb_func函数，在项目中真正的使用需要定义cb_func函数，来处理超时的连接
#include<queue>
#include<vector>
#include<iostream>
#include<assert.h>
#include<time.h>
#include"heaptimer.hpp"

using namespace std;

int main() {

    HeapTimer ht;

    for (int i = 0; i < 10; i++) {
        TimerNode node(i, time(nullptr), nullptr);
        ht.push(node);
    }

    ht.addExpireTime(1, time(nullptr) + 10);//调整非叶子节点
    ht.addExpireTime(5, time(nullptr) + 10);//调整叶子节点

    for (int i = 0; i < 10; i++) {
        TimerNode node = ht.top();
        cout << "sockfd = " << node.sockfd << " expire time = " << node.expire_time << "\n";
        ht.pop();
    }

    return 0;
}