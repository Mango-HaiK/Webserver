#include "httpconnection.h"

using namespace std;

const char* HttpConnection::g_srcDir;
std::atomic<int> HttpConnection::g_userCount;
bool HttpConnection::g_isET;

HttpConnection::HttpConnection() : m_fd(-1), m_addr({0}), 
                                    m_isClose(true)
{
}

HttpConnection::~HttpConnection()
{
    Close();
}

void HttpConnection::Init(int fd, const sockaddr_in& addr)
{
    assert(fd > 0);
    g_userCount++;
    m_addr = addr;
    m_fd = fd;
    m_writeBuff.RetrieveAll();
    m_readBuff.RetrieveAll();
    m_isClose = false;
    LOG_INFO("Client[%d](%s,%d) join, userCount: %d",
            m_fd,GetIP(), GetPort(), static_cast<int>(g_userCount));
}

int HttpConnection::GetFd() const
{
    return m_fd;
}

sockaddr_in HttpConnection::GetAddr() const
{
    return m_addr;
}

const char* HttpConnection::GetIP() const
{
    return inet_ntoa(m_addr.sin_addr);
}

int HttpConnection::GetPort() const
{
    return m_addr.sin_port;
}

void HttpConnection::Close()
{
    m_response.UnmapFile();
    if (m_isClose == false)
    {
        m_isClose = true;
        g_userCount--;
        close(m_fd);
        LOG_INFO("Client[%d](%s,%d) quit, userCount: %d",
            m_fd,GetIP(), GetPort(), static_cast<int>(g_userCount));
    }
}

ssize_t HttpConnection::Read(int* saveErrno)
{
    
}