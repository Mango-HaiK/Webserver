#ifndef SQLCONNPOOL_H_
#define SQLCONNPOOL_H_

#include <queue>
#include <mutex>
#include <assert.h>
#include <mysql/mysql.h>
#include <semaphore.h>
#include "../Log/log.h"

class SqlConnPool
{
public:
    //单例
    static SqlConnPool& getInstace();

    //初始化
    void Init(const char* host,const int port,
                const char* username, const char* password,
                const char* dbname,const int connSize);

    //获取一个连接
    MYSQL* getConn();

    //空闲连接入队
    void freeConn(MYSQL* conn);
    
    //获取空闲连接数
    int getFreeConnCount();

    //关闭连接池
    void ClosePool();

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN;   //最大连接数
    int m_usedConn; //使用的连接
    int m_freeConn; //空闲的连接

    std::queue<MYSQL*> m_connQue;  //等待队列
    std::mutex m_mutex;   
    sem_t m_sem;                   //管理队列信号量    
};

#endif  //SQLCONNPOOL_H_