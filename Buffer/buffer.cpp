#include"buffer.hpp"

Buffer::Buffer(size_t initBufferSize):buffer_(initBufferSize),readPos_(0),writePos_(0){}

size_t Buffer::WritableBytes()const{
    return buffer_.size()-writePos_;
}

size_t Buffer::ReadableBytes()const{
    return writePos_-readPos_;
}

size_t Buffer::PrependableBytes()const{
    return readPos_;
}

const char* Buffer::Peek()const{
    if(ReadableBytes())
        return BeginPtr_()+readPos_;
    return nullptr;
}

const char* Buffer::BeginWriteConst()const{
    if(WritableBytes())
        return BeginPtr_()+writePos_;
    return nullptr;
}

char* Buffer::BeginWrite(){
    if(WritableBytes())
        return BeginPtr_()+writePos_;
    return nullptr;
}

void Buffer::HasWritten(size_t len){
    writePos_ += len;
}

void Buffer::Retrieve(size_t len){
    assert(len <= ReadableBytes());
    readPos_ += len;
}

void Buffer::RetrieveUntil(const char* end){
    assert(Peek() <= end);
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll(){
    memset(&buffer_[0],0,buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}
    
string Buffer::RetrieveAllToStr(){
    string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

void Buffer::EnsureWriteable(size_t len){
    if(WritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}

void Buffer::Append(const string& str){
    Append(str.data(),str.length());
}

void Buffer::Append(const char* str, size_t len){
    assert(str);
    EnsureWriteable(len);
    copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const void* data, size_t len){
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const Buffer& buff){
    Append(buff.Peek(), buff.ReadableBytes());
}

ssize_t Buffer::ReadFd(int fd, int* Errno){
    // char testBuff[65535];
    // cout << "测试读：\n";
    // int str_len = read(fd,testBuff,65535);
    // printf("%s",testBuff);
    // cout << "测试读结束\n";

    char buff[65535];//64KB
    struct iovec iov[2];
    const size_t writable = WritableBytes();
    /* 分散读，先读到buffer_中，如果buffer_满了,在读到buff中,保证数据全部读完 */
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    // cout << "读取的长度：" << len <<"\n";
    // cout << "备用缓冲中的数据：\n";
    // cout << buff;
    // cout << "备用缓冲结束\n";
    if(len < 0) {
        *Errno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {
        writePos_ += len;
    }
    else {
        writePos_ = buffer_.size();
        Append(buff, len - writable);
    }
    // cout << "读缓冲中的内容：\n";
    // for(char c : buffer_){
    //     cout << c;
    // }
    // cout<<"读缓冲结束\n";
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* Errno){
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *Errno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}

char* Buffer::BeginPtr_(){
    return &(*buffer_.begin());
}

const char* Buffer::BeginPtr_() const {
    return &(*buffer_.begin());
}

void Buffer::MakeSpace_(size_t len){
    if(WritableBytes() + PrependableBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } 
    else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}

void Buffer::getContent(){
    cout << "缓冲区的内容(开始):\n";
    for(char c : buffer_)
        cout << c;
    cout << "缓冲区的内容(结束):\n";
}