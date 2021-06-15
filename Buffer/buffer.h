#ifndef BUFFER_H_
#define BUFFER_H_

#include <iostream>
#include <sys/uio.h> //readv
#include <unistd.h>  //write
#include <cstring>  
#include <vector>
#include <atomic>   //atomic
#include <assert.h>

#define MAX_BUFF_LEN 65535

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
    
    //可写字节数
    size_t WritableBytes() const;
    //可读字节数
    size_t ReadableBytes() const;
    size_t PrepenableBytes() const;
    
    //获取当前buff里的内容
    const char* Peek() const;
    //确保有位置存放要写入的数据
    void EnsureWriteble(size_t len);
    
    void HasWritten(size_t len);    //
    
    //追加的位置，也是前段数据的结尾处+1
    char* BeginWrite();
    const char* BeginWriteConst() const;

    //恢复
    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);
    void RetrieveAll();
    //返回字符串类型的内容，并重置buff的内容
    std::string RetrieveToStr();
    
    //追加数据
    void Append(const std::string& str);
    void Append(const void* data,size_t len);
    void Append(const char* str,size_t len);
    void Append(const Buffer& buff);

    //读
    ssize_t ReadFd(int fd,int* Errno);
    ssize_t WriteFd(int fd,int* Errno);
};

#endif //BUFFER_H