#ifndef SQLCONNRAII_H_
#define SQLCONNRAII_H_
#include "sqlConnPool.h"

class sqlConnRAII
{
private:
    MYSQL *m_sql;
    SqlConnPool* m_connpool;
public:
    sqlConnRAII(MYSQL **sql, SqlConnPool *connpool) {
        assert(connpool);
        *sql = connpool->getConn();
        m_sql = *sql;
        m_connpool = connpool;
    }
    ~sqlConnRAII() {
        if (m_sql)
        {
            m_connpool->freeConn(m_sql);
        } 
    }
};

#endif