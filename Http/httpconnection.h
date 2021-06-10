#ifndef HTTPCONNECTION_H_
#define HTTPCONNECTION_H_

#include <arpa/inet.h>  //sockaddr_in
#include <atomic>

#include "httprequest.h"
#include "httpresponse.h"
#include "../Buffer/buffer.h"
class HttpConnection
{
public:
    HttpConnection();
    ~HttpConnection();

    /*初始化socket*/
    void Init(int, const sockaddr_in&);

    /**/
    ssize_t Read(int*);
    
    ssize_t Write(int*);

    /*获取该连接的socket描述符*/
    int GetFd() const;

    /*获取该连接的Port*/
    int GetPort() const;

    /*获取该连接的IP*/
    const char* GetIP() const;

    /*获取该连接的地址*/
    sockaddr_in GetAddr() const;

    bool process();

    void Close();

    /*ET模式*/
    static bool g_isET;
    
    static const char* g_srcDir;
    
    /*用户总数*/
    static std::atomic<int> g_userCount;

private:
    int m_fd;

    sockaddr_in m_addr;
    
    bool m_isClose;

    int m_iovCnt;
    iovec m_iov[2];

    /*读缓冲区*/
    Buffer m_readBuff;
    /*写缓冲区*/
    Buffer m_writeBuff;
    
    HttpRequest m_request;
    HttpResponse m_response;
};

#endif