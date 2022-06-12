#include<iostream>

template<typename T,typename F>
T use_f(T v,F f){
    static int count = 0;
    count++;
    std::cout<<" use_f count = "<<count<<", &count = "<<&count<<std::endl;
    return f(v);
}

class Fp{
public:
    Fp(double z = 1.0):z_(z){}
    double operator()(double q){return z_+q;}
private:
    double z_;
};

class Fq{
public:
    Fq(double z = 1.0):z_(z){}
    double operator()(double q){return z_+q;}
private:
    double z_;
};

double dup(double x){return 2.0*x;}
double square(double x){return x*x;}

int main(){
    using std::cout;
    using std::endl;

    double y = 1.21;
    cout<<"Function pointer dub:\n";
    cout<<" "<<use_f(y,dup)<<endl;
    cout<<"Function pointer square:\n";
    cout<<" "<<use_f(y,square)<<endl;
    cout<<"Function object Fp:\n";
    cout<<" "<<use_f(y,Fp(5.0))<<endl;
    cout<<"Function object Fq:\n";
    cout<<" "<<use_f(y,Fq(5.0))<<endl;
    cout<<"Lambda expression 1:\n";
    cout<<" "<<use_f(y,[](double u){return u*u;})<<endl;
    cout<<"Lambda expression 2:\n";
    cout<<" "<<use_f(y,[](double u){return u+u/2.0;})<<endl;

    return 0;
}