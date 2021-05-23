
#include "log.h"

Log::Log():m_lineCount(0),m_toDay(0),
            m_isOpen(false),m_isAsync(false),
            m_deque(nullptr),m_writeThread(nullptr)            
{

}

Log::~Log()
{   
    //销毁线程以及处理完队列中的数据
    if (m_writeThread->joinable() && m_writeThread)
    {
        while (!m_deque->Empty())
        {
            m_deque->flush();
        }
        m_deque->Close();
        m_writeThread->join();
    }
    if (m_fd)
    {
        lg_mutex locker(m_mutex);
        flush();        //TODO
        fclose(m_fd);
    }
}

void Log::Init(int level = 1,const char* path,
                const char* suffix,
                size_t maxCapacity)
{
    m_isOpen = true;
    m_level = level;
    if (maxCapacity > 0)
    {
        m_isAsync = true;
        if (!m_deque)
        {
            std::unique_ptr<Blockdeque<std::string> > newDeque(new Blockdeque<std::string>());
            //将newDeque对内容移动到m_deque，此过程不会复制因此开销低，但之后newDeque可能为空
            m_deque = move(newDeque);

            std::unique_ptr<std::thread> newThread(new std::thread(FlushLogThread));
            m_writeThread = move(newThread);
        }else
        {
            m_isAsync = false;
        }
        
        m_lineCount = 0;
        m_path = path;
        m_suffix = suffix;
        //时间
        time_t timer = time(nullptr);
        tm *t = localtime(&timer);
        //格式化日志文件名
        char fileName[LOG_NAME_LEN] = {0};
        snprintf(fileName,LOG_NAME_LEN - 1,"%s/%04d_%02d_%02d%s",
                    fileName,t->tm_year,t->tm_mon,t->tm_mday,m_suffix);
        m_toDay = t->tm_mday;

        {
            lg_mutex locker(m_mutex);
            m_buff
        }
    }
    
}

void Log::SetLevel(int level)
{
    lg_mutex locker(m_mutex);
    this->m_level = level;
}

int Log::GetLevel()
{
    lg_mutex locker(m_mutex);
    return m_level;
}

Log* Log::Instance()
{
    static Log instance;
    return &instance;
}

void Log::__AsyncWrite ()
{
    std::string str = "";
    while (m_deque->pop(str))
    {
        lg_mutex locker(m_mutex);
        fputs(str.c_str(),m_fd);
    }
}

void Log::FlushLogThread()
{
    Log::Instance()->__AsyncWrite();
}
