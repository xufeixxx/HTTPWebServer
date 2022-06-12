#include"HTTPRequest.hpp"

HTTPRequest::HTTPRequest():state_(REQUEST_LINE){}

void HTTPRequest::init(){
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

string HTTPRequest::path()const{
    return path_;
}

string HTTPRequest::method()const{
    return method_;
}

string HTTPRequest::version()const{
    return version_;
}

string HTTPRequest::GetPost(const string& key) const{
    assert(!key.empty());
    auto it = post_.find(key);
    if(it != post_.end())
        return it->second;
    return "";
}
    
bool HTTPRequest::IsKeepAlive() const{
    auto it = header_.find("Connection");
    if(it != header_.end())
        return it->second == "keep-alive";
    return false;
}

bool HTTPRequest::ParseRequestLine_(const string& line){
    //cout << "request line = "<<line<<endl;
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if(regex_match(line,subMatch,patten)){
        method_ = subMatch[1];
        path_ = subMatch[2];
        if(path_.back() == '?'){
            path_.pop_back();
        }
        //cout << "path_ = "<<path_<<endl;
        version_ = subMatch[3];
        state_ = HEADER;
        return true;
    }
    LOG_ERROR("RequestLine Error\n");
    return false;
}

void HTTPRequest::ParseHeader_(const string& line){
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        header_.insert(make_pair(subMatch[1],subMatch[2]));
    }
    else {
        state_ = BODY;
    }
}

void HTTPRequest::ParseBody_(const string& line){
    body_ = line;
    if(method_ == "POST")
        ParsePost_();
    state_ = FINISH;
}

void HTTPRequest::ParsePath_(){//解析action
    //cout << "path_ = "<<path_<<endl;
    if(path_ == "/" || path_ == "/login_html")
        path_ = "/login.html";
    else if(path_ == "/register_html")
        path_ = "/register.html";
    //cout << "path_ = "<<path_<<endl;
}

void HTTPRequest::ParsePost_(){
    string newbody = body_+"&";
    string::size_type start_index = 0;
    string::size_type n;
    while((n = newbody.find("&",start_index)) != string::npos){
        string sub(newbody.begin()+start_index,newbody.begin()+n);
        string::size_type pos = sub.find("=");
        post_.insert(make_pair(string(sub.begin(),sub.begin()+pos),string(sub.begin()+pos+1,sub.end())));
        start_index = n+1;
    }

    //cout << "ParsePost_:" << " 用户名：" << post_["username"] <<" 密码：" << post_["pwd"] <<endl;

    if(path_ == "/login"){

        if(loginVerify(post_["username"],post_["pwd"])){
            //cout << "验证成功" <<endl;
            path_ = "/logsuccess.html";
        }else{
            path_ = "/logerror.html";
        }

    }else if(path_ == "/register"){

        if(registerUser(post_["username"],post_["pwd"],post_["gender"],post_["address"],post_["occupation"])){
            path_ = "/registersuccess.html";
        }else{
            path_ = "/registererror.html";
        }
    }
    
}

bool HTTPRequest::loginVerify(const string& username,const string& password){
    if(username.empty() || password.empty())return false;
    bool login_verify = true;
    MYSQL* conn;
    SQLConnRAII(&conn,SQLConnPool::getInstance());
    assert(conn);
    char sql_order[256] = {0};
    snprintf(sql_order,256,"SELECT * FROM user WHERE username = '%s' AND password = '%s'",username.c_str(),password.c_str());
    if(mysql_query(conn,sql_order)){
        return false;
    }else{
        MYSQL_RES* result = mysql_store_result(conn);
        // printf("结果集中行的数量是%d\n",static_cast<int>(mysql_num_rows(result)));
        // printf("结果集中列的数量是%d\n",mysql_num_fields(result));
        if(mysql_num_rows(result)){
            login_verify = true;
        }else{
            //查询结果为0
            login_verify = false;
        }
        mysql_free_result(result);
    } 
    //SQLConnPool::getInstance()->releaseConnection(conn);
    return login_verify;
}

bool HTTPRequest::registerUser(const string& username,const string& password,const string& gender,const string& address,const string& occupation){
    if(username.empty() || password.empty())return false;
    //bool register_verify = true;
    MYSQL* conn;
    SQLConnRAII(&conn,SQLConnPool::getInstance());
    assert(conn);
    char sql_order[256] = {0};
    snprintf(sql_order,256,"INSERT INTO user(username,password,gender,address,occupation) VALUES('%s','%s','%s','%s','%s')",
    username.c_str(),password.c_str(),gender.c_str(),address.c_str(),occupation.c_str());
    if(mysql_query(conn,sql_order)){
        return false;
    }
    return true;
} 

bool HTTPRequest::parse(Buffer& buffer){
    const char CRLF[] = "\r\n";//行结束标记
    if(buffer.ReadableBytes() <= 0) {
        return false;
    }
    while(buffer.ReadableBytes() && state_ != FINISH) {
        /*search函数查找如果没找到返回查找范围的超尾,对应下面函数就是buffer.BeginWriteConst()*/
        const char* lineEnd = search(buffer.Peek(), buffer.BeginWriteConst(), CRLF, CRLF + 2);
        string line(buffer.Peek(), lineEnd);
        //cout << "parse = "<<line<<"当前线程为:"<<pthread_self()<<"\n";
        switch(state_)
        {
        case REQUEST_LINE:
            if(!ParseRequestLine_(line)) {
                return false;
            }
            ParsePath_();
            break;    
        case HEADER:
            ParseHeader_(line);
            if(buffer.ReadableBytes() <= 2) {
                state_ = FINISH;
            } 
            break;
        case BODY:
            ParseBody_(line); 
            break;
        default:
            break;
        }
        if(lineEnd == buffer.BeginWrite()) { break; }//search函数没有找到\r\n
        buffer.RetrieveUntil(lineEnd + 2);
    }
    //cout <<"解析请求结束\n";
    LOG_DEBUG("[%s], [%s], [%s]\n", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}