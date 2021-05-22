#ifndef BUFFER_H_
#define BUFEER_H_

#include <iostream>
#include <sys/uio.h> //readv、writev
#include <cstring>
#include <vector>
#include <atomic>

class Buffer
{
private:

    char *__BeginPtr();
    const char* __BeginPtr() const;
    void __MakeSpace(size_t len);

    std::vector<char> m_buffer;
    //原子数据类型
    std::atomic<size_t> m_readPos;
    std::atomic<size_t> m_writePos;
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;
    
    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrepenableBytes() const; //前置内容
    
    const char* Peek() const;
    void EnsureWriteble(size_t len);//保证写
    void HasWritten(size_t len);    //
    
    //追加数据
    void Append(const std::string& str);
    void Append(const void* data,size_t len);
    void Append(const char* str,size_t len);
    void Append(const Buffer& buff);

    //读
    ssize_t ReadFd(int fd,int* Errno);
    ssize_t WriteFd(int fd,int* Errno);
    

};

#endif //BUFFER_H_