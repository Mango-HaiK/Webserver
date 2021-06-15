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

int HttpConnection::ToWriteBytes() {
    return m_iov[0].iov_len + m_iov[1].iov_len;
}

bool HttpConnection::IsKeepAlive() const
{
    return m_request.IsKeepAlive();
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
    ssize_t len = -1;
    do
    {
        len = m_readBuff.ReadFd(m_fd, saveErrno);
        if (len <= 0)
            break;
    } while (g_isET);
    return len;
}

ssize_t HttpConnection::Write(int* saveErrno)
{
    ssize_t len = -1;
    do
    {
        len = writev(m_fd, m_iov, m_iovCnt);
        if (len <= 0)
        {
            *saveErrno = errno;
            break;
        }
        if (m_iov[0].iov_len + m_iov[1].iov_len == 0)
        {
            break;
        }
        else if (static_cast<size_t>(len) > m_iov[0].iov_len)
        {
            m_iov[1].iov_base = (uint8_t*)m_iov[0].iov_base + (len - m_iov[0].iov_len);
            m_iov[1].iov_len -= (len - m_iov[0].iov_len);
            if (m_iov[0].iov_len)
            {
                m_writeBuff.RetrieveAll();
                m_iov[0].iov_len = 0;
            }
        }
        else
        {
            m_iov[0].iov_base = (uint8_t*)m_iov[0].iov_base + len;
            m_iov[0].iov_len -= len;
            m_writeBuff.Retrieve(len);
        }
    } while (g_isET || (m_iov[0].iov_len + m_iov[1].iov_len) > 10240);
    return len;
}

bool HttpConnection::process()
{
    m_request.Init();
    if (m_readBuff.ReadableBytes() <= 0)
    {
        return false;
    }
    /*匹配成功*/
    else if(m_request.Parse(m_readBuff))
    {
        LOG_DEBUG("%s", m_request.path().c_str());
        m_response.Init(g_srcDir, m_request.path(), m_request.IsKeepAlive(), 200);
    }else
    {
        m_response.Init(g_srcDir, m_request.path(), false, 400);
    }
    m_response.MakeResponse(m_writeBuff);

    m_iov[0].iov_base = const_cast<char*>(m_writeBuff.Peek());
    m_iov[0].iov_len = m_writeBuff.ReadableBytes();
    m_iovCnt = 1;

    if (m_response.FileLen() > 0 && m_response.File())
    {
        m_iov[1].iov_base = m_response.File();
        m_iov[1].iov_len = m_response.FileLen();
        m_iovCnt = 2;
    }
    LOG_DEBUG("filesize:%d, %d to %d", m_response.FileLen(), m_iovCnt, m_iov[0].iov_len + m_iov[1].iov_len)
    return true;
}