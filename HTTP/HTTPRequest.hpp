#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include<string>
#include<unordered_map>
#include<algorithm>
#include<assert.h>
#include<regex>
#include<mysql/mysql.h>
#include<iostream>

#include"../Buffer/buffer.hpp"
#include"../Log/log.hpp"
#include"../SQLConnPool/sqlconnRAII.hpp"

using namespace std;

class HTTPRequest{
public:
    enum PARSE_STATE{
        REQUEST_LINE,
        HEADER,
        BODY,
        FINISH,
    };

    HTTPRequest();
    void init();

    bool parse(Buffer& buffer);

    string path() const;
    string method() const;
    string version() const;
    string GetPost(const string& key) const;
    bool IsKeepAlive() const;


private:
    bool ParseRequestLine_(const string& line);
    void ParseHeader_(const string& line);
    void ParseBody_(const string& line);

    void ParsePath_();
    void ParsePost_();

    static bool loginVerify(const string& username,const string& password);
    static bool registerUser(const string& username,const string& password,
    const string& gender,const string& address,const string& occupation); 

    PARSE_STATE state_;
    string method_;
    string path_;
    string version_;
    string body_;
    unordered_map<string,string> header_;
    unordered_map<string,string> post_;

};


#endif