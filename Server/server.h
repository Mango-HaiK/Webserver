#ifndef SERVER_H_
#define SERVER_H_

#include <sys/socket.h>     //socket
#include <netinet/in.h>     //sockaddr_in
#include <arpa/inet.h>

#include "../Log/log.h"
#include "../Pool/sqlConnRAII.h"
#include "../Pool/threadPool.h"
#include "../epoll/epoller.h"
#include "../Http/httpconnection.h"
#include "../Timer/timer.h"

#define DATABASEIP "localhost"

class Server
{
public:
    /*端口号，触发模式，超时时间，是否等待数据处理完再关闭，数据库端口号，
    数据库用户名，数据库密码，数据库名，连接池数目，线程数目，是否开启日志，
    日志等级，日志队列大小*/
    Server(
        int port, int trigMode, int timeoutMS, bool isWaitClose,
        int sqlPort,const char* sqlUser,const char* sqlpwd,
        const char* dbName, int connPoonum, int threadNum,
        bool openLog, int logLevel, int logQueSize);    
    ~Server();
    /*开启服务*/
    void Start();
private:
    /*初始化socket*/
    bool __InitSocket();

    /*初始化事件模式*/
    void __InitEventMode(int trigMode);

    /*客户连接添加*/
    void __AddClient(int fd, sockaddr_in addr);
    
    void __DealListen();
    void __Dealwrite(HttpConnection* client);
    void __DealRead(HttpConnection* client);

    void __SendError(int fd, const* info);
    void __ExtentTime(HttpConnection* client);
    void __CloseConn(HttpConnection* client);

    void __OnRead(HttpConnection* client);
    void __OnWrite(HttpConnection* client);
    void __OnProcess(HttpConnection* client);

    static const int MAX_FD = 65535;

    /*设置非阻塞fd*/
    static int SetNonblockFd(int fd);

    int m_port;

    /*等待所有数据发送完或超时*/
    bool m_isWaitClose;

    int m_timeoutMS;
    bool m_isClose;
    int m_listenFd;
    char* m_srcDir;

    uint32_t m_listenEvent;
    uint32_t m_connEvent;

    std::unique_ptr<HeapTimer> m_timer;
    std::unique_ptr<ThreadPool> m_threadpool;
    std::unique_ptr<Epoller> m_epoller;
    std::unordered_map<int, HttpConnection> m_user;
};

#endif 