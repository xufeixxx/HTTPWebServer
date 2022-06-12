#ifndef BUFFER_H
#define BUFFER_H

#include<iostream>
#include<vector>
#include<atomic>
#include<assert.h>
#include<string.h>
#include<string>
#include<unistd.h>
#include<sys/uio.h>

using namespace std;

using buffer_index = unsigned int;

class Buffer{
public:
    Buffer(size_t initBufferSize = 1024);

    size_t WritableBytes() const;//还有多少个空间可以去写       
    size_t ReadableBytes() const ;//还有多少char需要去读
    size_t PrependableBytes() const;//已经读取的字符

    const char* Peek() const;//下一个将要从缓冲区中读取的字符的指针
    const char* BeginWriteConst() const;//返回下一个要写入数据的空闲空间的指针
    char* BeginWrite();//返回下一个要写入数据的空闲空间的指针

    void HasWritten(size_t len);//writePos_ += len
    void Retrieve(size_t len);//readPos_ += len
    void RetrieveUntil(const char* end);//移动readPos_直到指向end所指的元素
    void RetrieveAll() ;//表示检索完所有元素，buffer_、readPos_和writePos_全部置为0
    std::string RetrieveAllToStr();//将buffer_中所有的char封装成一个string并返回，同时调用RetrieveAll()函数

    void EnsureWriteable(size_t len);//这个函数用来确保buffer_中有len个空闲区与可写

    /*将各种形式的字符串数据放到缓冲区中，确保可以全部放进buffer_中*/
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);//将fd中的数据读到buffer_中
    ssize_t WriteFd(int fd, int* Errno);//将buffer_中的数据写到fd中 ssize_t = long int

    void getContent();

private:
    char* BeginPtr_();//返回缓冲区首个位置的指针
    const char* BeginPtr_() const;//返回缓冲区收个位置的指针
    void MakeSpace_(size_t len);//制造至少len个可存储的空闲位置

    vector<char> buffer_;//使用vector当作缓冲区

    /*原子类型，针对原子类型操作要不一步完成，
    要么不做，不可能出现操作一半被切换CPU，这样防止由于多线程指令交叉执行带来的可能错误。*/
    atomic<buffer_index> readPos_;
    atomic<buffer_index> writePos_;
};

#endif