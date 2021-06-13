#include "server.h"

Server::Server(
        int port, int trigMode, int timeoutMS, bool isWaitClose,
        int sqlPort,const char* sqlUser,const char* sqlpwd,
        const char* dbName, int connPoonum, int threadNum,
        bool openLog, int logLevel, int logQueSize):
        m_port(port),m_timeoutMS(timeoutMS),m_isWaitClose(false),
        m_timer(new HeapTimer()), m_threadpool(new ThreadPool(threadNum)),
        m_epoller(new Epoller())
{
    //获取当前工作目录
    getcwd(m_srcDir, 256);
    assert(m_srcDir);
    strncat(m_srcDir, "/resources/", 16);
    HttpConnection::g_userCount = 0;
    HttpConnection::g_srcDir = m_srcDir;
    SqlConnPool::getInstace()->Init(DATABASEIP,sqlPort,sqlUser,
                                 sqlpwd, dbName, connPoonum);

    __InitEventMode(trigMode);

    if (!__InitSocket())
    {
        m_isClose = true;
    }
    
    if (openLog)
    {
        Log::Instance()->Init(logLevel, "./log",".log", logQueSize);
        if(m_isClose)
        {
            LOG_ERROR("******* SERVER INIT ERROR! *******");
        }else
        {
            LOG_INFO("****** SERVER INIT *******");
            LOG_INFO("Port: %d, OpenLinger: %s",m_port, isWaitClose ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                        (m_listenEvent & EPOLLET ? "ET" : "LT"),
                        (m_connEvent & EPOLLET ? "ET" : "LT"));
            LOG_INFO("Log level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConnection::g_srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d",connPoonum, threadNum);
        }
    }
}

Server::~Server()
{

}

void Server::__InitEventMode(int trigMode)
{
    /*在socket上接收到对方关闭的请求之后触发-读挂断*/
    m_listenEvent = EPOLLRDHUP;
    /*一个连接事件只能被一个线程处理，
      EPOLLONESHOT-在重置之前最多触发一个事件，且触发一次，*/
    m_connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        m_connEvent |= EPOLLET;
        break;
    case 2:
        m_listenEvent |= EPOLLET;
        break;
    default:
        m_listenEvent |= EPOLLET;
        m_connEvent |= EPOLLET;
        break;
    }
    HttpConnection::g_isET = (m_connEvent & EPOLLET);
}

bool Server::__InitSocket()
{
    int ret;
    sockaddr_in addr;
    if (m_port > 65535 || m_port < 1024)
    {
        LOG_ERROR("Port: %d error!", m_port);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    linger optLinger = {0};
    if (m_isWaitClose)
    {
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    
    m_listenFd = socket(AF_INET, SOCK_STREAM,0);
    if (m_listenFd < 0)
    {
        LOG_ERROR("Create socket error!",m_port);
        return false;
    }

    /*设置socketFd的选项为如果有数据未完成发送则等待处理结束*/
    ret = setsockopt(m_listenFd, SOL_SOCKET,
                    SO_LINGER,(const void*)&optLinger,sizeof(optLinger));
    if (ret == -1)
    {
        close(m_listenFd);
        LOG_ERROR("Init linger error!");
        return false;
    }
    
    /*端口复用*/
    int optval = 1;
    ret = setsockopt(m_listenFd, SOL_SOCKET,
                     SO_REUSEADDR,(const void*)&optval,sizeof(optval));
    if (ret < 0)
    {
        LOG_ERROR("set socket SO_REUSEADDR error!");
        close(m_listenFd);
        return false;
    }
    
    ret = bind(m_listenFd,(sockaddr*)&addr,sizeof(addr));
    if (ret < 0)
    {
        LOG_ERROR("bind port: %d error!",m_port);
        close(m_listenFd);
        return false;
    }

    ret = listen(m_listenFd, 10);
    if (ret < 0)
    {
        LOG_ERROR("listen port: %d error!",m_port);
        close(m_listenFd);
        return false;
    }
    
    ret = m_epoller->AddFd(m_listenFd, m_listenEvent | EPOLLIN);
    if (!ret)
    {
        LOG_ERROR("Add listen error!");
        close(m_listenFd);
        return false;
    }
    SetNonblockFd(m_listenFd);
    LOG_INFO("Server port: %d", m_port);
    return true;
}

int Server::SetNonblockFd(int fd)
{
    int newfd = fcntl(fd, F_GETFD, 0);
    newfd |= O_NONBLOCK;
    newfd = fcntl(fd, F_SETFL, newfd);
    return newfd;
}

void Server::Start()
{
    /*默认阻塞等待*/
    int timeMS = -1;
    if (!m_isClose)
    {
        LOG_INFO("******* Server Start *******");
    }
    while (!m_isClose)
    {
        if (m_timeoutMS > 0)
        {
            timeMS = m_timer->getNextTick();
        }
        int eventCnt = m_epoller->Wait(timeMS);
        for (int i = 0; i < eventCnt; i++)
        {
            /*获取事件并分别对事件进行处理*/
            int fd = m_epoller->GetEventFd(i);
            uint32_t events = m_epoller->GetEvents(i);
            if (fd == m_listenFd) //新连接
            {
                __DealListen();
            }
            /*连接关闭*/
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                __CloseConn(&m_user[fd]);
            }
            /*事件写入*/
            else if (events & EPOLLIN)
            {
                __DealRead(&m_user[fd]);
            }
            /*事件读入*/
            else if (events & EPOLLOUT)
            {
                __Dealwrite(&m_user[fd]);
            }else
            {
                LOG_ERROR("Unexpected event!");
            }
        }      
    }
}

void Server::__SendError(int fd, const char* info)
{
    assert(fd > 0);
    //TODO
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0)
    {
        LOG_WARN("send error to client[%d] error!",fd);
    }
    close(fd);
}

void Server::__CloseConn(HttpConnection* client)
{
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd);
    m_epoller->DelFd(client->GetFd());
    client->Close();
}

void Server::__AddClient(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    m_user[fd].Init(fd, addr);
    /*定时器添加*/
    if (m_timeoutMS > 0)
    {
        //超时关闭连接
        m_timer->add(fd, m_timeoutMS, std::bind(&Server::__CloseConn, this, &m_user[fd]));
    }
    m_epoller->AddFd(fd, EPOLLIN | m_connEvent);
    SetNonblockFd(fd);
    LOG_INFO("Client[%d] join!", m_user[fd].GetFd());
}

void Server::__DealListen()
{
    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    do
    {
        int fd = accept(m_listenFd,(sockaddr*)&client_addr,&len);
        if (fd <= 0)
        {
            return;
        }
        else if(HttpConnection::g_userCount >= MAX_FD)
        {
            __SendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        __AddClient(fd, client_addr);
    } while (m_listenFd & EPOLLET);
}

void Server::__ExtentTime(HttpConnection* client)
{
    assert(client);
    if (m_timeoutMS > 0)
    {
        m_timer->adjust(client->GetFd(), m_timeoutMS);
    }
}

void Server::OnProcess(HttpConnection* client)
{
    if (client->process())
    {
        
    }
    
}

void Server::__OnRead(HttpConnection* client)
{
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->Read(&readErrno);
    /*当fd无数据可读时read会将errno设置为EAGAIN*/
    if (ret <= 0 && readErrno != EAGAIN)
    {
        __CloseConn(client);
        return;
    }
    OnProcess(client);
}

void Server::__DealRead(HttpConnection* client)
{
    assert(client);
    /*连接还活着，需要重置超时时间*/
    __ExtentTime(client);
    m_threadpool->AddTask(std::bind(&Server::__OnRead,this,client));
}