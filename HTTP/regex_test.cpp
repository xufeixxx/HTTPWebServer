#include<regex>
#include<iostream>
#include<string>
#include<unordered_map>

using namespace std;
int main(){

    string str = "GET /login_html HTTP/1.1";
    cout<< str <<endl;
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;

    bool isMatch = regex_match(str,subMatch,patten);

    cout<<subMatch[1]<<endl;
    cout<<subMatch[2]<<endl;
    cout<<subMatch[3]<<endl;

    unordered_map<string,string> map;
    string str2 = "Content-Length: 21";
    regex patten2("^([^:]*): ?(.*)$");
    smatch subMatch2;
    bool isMatch2 = regex_match(str2,subMatch2,patten2);
    cout<< subMatch2[1] <<endl;
    cout<< subMatch2[2] <<endl;
    map.insert(make_pair(subMatch2[1],subMatch2[2]));
}