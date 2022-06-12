#include<time.h>
#include<iostream>

using namespace std;

int main(){

    time_t cur = time(nullptr);
    cout<<"计时开始"<<endl;
    while(1){
        if(time(nullptr) - cur == 10){
            cout<<"过了十秒钟"<<endl;
            cur = time(nullptr);
        }
    }
}