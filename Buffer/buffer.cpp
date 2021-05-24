#include "buffer.h"

Buffer::Buffer(int initBufferSize):
                m_buffer(initBufferSize),
                m_readPos(0),m_writePos(0){}

size_t Buffer::ReadableBytes() const
{
    return m_writePos - m_readPos;
}

size_t Buffer::WritableBytes() const
{
    return m_buffer.size() - m_writePos;
}

size_t Buffer::PrepenableBytes() const
{
    return m_readPos;
}

const char* Buffer::Peek() const
{
    return __BeginPtr() + m_readPos;
}

void Buffer::Retrieve(size_t len)
{
    assert(len <= ReadableBytes());
    m_readPos += len;
}

void Buffer::RetrieveUntil(const char* end)
{
    assert(Peek() <= end);
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll()
{
    bzero(&m_buffer[0],m_buffer.size());
    m_readPos = 0;
    m_writePos = 0;
}

std::string Buffer::RetrieveToStr()
{
    std::string str(Peek(),ReadableBytes());
    RetrieveAll();
    return str;
}

const char* Buffer::BeginWriteConst() const
{
    return __BeginPtr() + m_writePos;
}

char* Buffer::BeginWrite()
{
    return __BeginPtr() + m_writePos;
}

void Buffer::HasWritten(size_t len)
{
    m_writePos += len;
}

void Buffer::Append(const std::string& str)
{
    Append(str.data(),str.size());
}

void Buffer::Append(const void* data, size_t len)
{
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const char* str, size_t len)
{
    assert(str);
    EnsureWriteble(len);
    std::copy(str, str+len, BeginWrite());
    HasWritten(len);
}
void Buffer::Append(const Buffer& buff)
{
    Append(buff.Peek(), buff.ReadableBytes());
}

char* Buffer::__BeginPtr()
{
    return &*m_buffer.begin();
}

const char* Buffer::__BeginPtr() const
{
    return &*m_buffer.begin();
}

void Buffer::EnsureWriteble(size_t len)
{
    if (WritableBytes() < len)
    {
        __MakeSpace(len);
    }
    assert(WritableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd,int *_errno)
{
    char buff[MAX_BUFF_LEN];
    iovec iov[2];
    const size_t writable = WritableBytes();

    //分散读
    iov[0].iov_base = __BeginPtr() + m_writePos;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd,iov,2);
    if(len < 0)
    {
        *_errno = errno;
    }
    else if(static_cast<size_t>(len) <= writable)
    {
        m_writePos += len;
    }else
    {
        m_writePos = m_buffer.size();
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd,int* _errno)
{
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(),readSize);
    if (len <0 )
    {
        *_errno = errno;
        return len;
    }
    m_readPos += len;
    return len;
}

void Buffer::__MakeSpace(size_t len)
{
    if (WritableBytes() + PrepenableBytes() < len)
    {
        m_buffer.resize(m_writePos + len + 1);   
    }else
    {
        size_t readable = ReadableBytes();
        std::copy(__BeginPtr() + m_readPos, __BeginPtr() + m_writePos, __BeginPtr());
        m_readPos = 0;
        m_writePos = m_readPos + readable;
        assert(readable == ReadableBytes());
    }
}