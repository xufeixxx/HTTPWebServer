#include<iostream>
#include<functional>

double my_divide(double x,double y){return x/y;}

struct MyPair{
    double a,b;
    double multiply(){return a*b;}
};

int main(){
    using namespace std::placeholders;

    auto fn_five = std::bind(my_divide,10,2);
    std::cout<<fn_five()<<std::endl;

    auto fn_half = std::bind(my_divide,_1,2);
    std::cout<<fn_half(10)<<"\n";

    MyPair ten_two{10,2};
    auto bound_member_fn = std::bind(&MyPair::multiply,_1);
    std::cout << bound_member_fn(ten_two) <<"\n";

    return 0;
}