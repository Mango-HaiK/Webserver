#include "sqlConnPool.h"

SqlConnPool::SqlConnPool() : m_usedConn(0), m_freeConn()
{
}

SqlConnPool::~SqlConnPool()
{
    ClosePool();
}

SqlConnPool& SqlConnPool::getInstace()
{
    static SqlConnPool connPool;
    return connPool;
}

void SqlConnPool::Init(const char* host,const int port,
            const char* username, const char* password,
            const char* dbname,const int connSize)
{
    for (int i = 0; i < connSize; i++)
    {
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if (!sql)
        {
            LOG_ERROR("MySQL init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, username,
                                 password,dbname,
                                 port,nullptr,0);
        if (!sql)
        {
            LOG_ERROR("MySQL connect error");
            assert(sql);
        }
        m_connQue.push(sql);
    }
    MAX_CONN = connSize;
    sem_init(&m_sem, 0, MAX_CONN);
}

MYSQL* SqlConnPool::getConn()
{
    MYSQL* sql = nullptr;
    if (m_connQue.empty())
    {
        LOG_WARN("MySQL is busy.");
        return nullptr;
    }
    sem_wait(&m_sem);
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        sql = m_connQue.front();
        m_connQue.pop();
    }
    return sql;
}

void SqlConnPool::freeConn(MYSQL* sql)
{
    assert(sql);
    lg_mutex locker(m_mutex);
    m_connQue.push(sql);
    sem_post(&m_sem);
}

void SqlConnPool::ClosePool()
{
    lg_mutex locker(m_mutex);
    while(!m_connQue.empty())
    {
        MYSQL* item = m_connQue.front();
        mysql_close(item);
        m_connQue.pop();
    }
    mysql_server_end();
}

int SqlConnPool::getFreeConnCount()
{
    lg_mutex locker(m_mutex);
    return m_connQue.size();
}