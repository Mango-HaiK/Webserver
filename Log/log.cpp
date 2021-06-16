#include "log.h"

Log::Log():m_lineCount(0),m_toDay(0),
            m_isOpen(false),m_isAsync(false),
            m_fd(nullptr), m_deque(nullptr),
            m_writeThread(nullptr)
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
        flush();        
        fclose(m_fd);
    }
}

void Log::Init(int level = 1,const char* path,
                const char* suffix,
                size_t maxDequeSize)
{   
    m_isOpen = true;
    m_level = level;
    //如果设置了队列大小则为异步
    if (maxDequeSize > 0)
    {
        m_isAsync = true;
        if (!m_deque)
        {
            //创建队列和处理线程
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
                    m_path,t->tm_year + 1900,t->tm_mon + 1,t->tm_mday,m_suffix);
        m_toDay = t->tm_mday;

        {
            lg_mutex locker(m_mutex);
            m_buff.RetrieveAll();
            if (m_fd)
            {
                //将缓冲区的内容加入文件
                flush();
                fclose(m_fd);
            }
            
            m_fd = fopen(fileName,"a");
            if (m_fd == nullptr)
            {
                mkdir(path,0777);           //第二个参数是访问权限
                m_fd = fopen(fileName,"a");
            }
            assert(m_fd != nullptr);
        }
    }
}

void Log::write(int level, const char* format, ...)
{
    timeval nowtime = {0, 0};
    gettimeofday(&nowtime,nullptr);     //1.返回当前距离1970年的秒数和毫秒数
    time_t tSec = nowtime.tv_sec;       //2.获取秒数
    tm *t = localtime(&tSec);           //3.得到一个存储日期的结构体
    va_list va_List;
    //不是同一天或者日志行数已经太多了,新建日志文件
    if (m_toDay != t->tm_mday || m_lineCount > MAX_LINES)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        locker.unlock();

        //设置日期
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail,36,"%04d_%02d_%02d",t->tm_year + 1900,t->tm_mon + 1,t->tm_mday);
        
        //如果是不是同一天
        if(m_toDay != t->tm_mday)
        {
            snprintf(newFile, LOG_NAME_LEN - 72,"%s/%s%s", m_path, tail, m_suffix);
            m_toDay = t->tm_mday;
            m_lineCount = 0;
        }else   //当天的行数已满
        {
            snprintf(newFile, LOG_NAME_LEN - 72,"%s/%s_%d%s",
                         m_path, tail, (m_lineCount / MAX_LINES), m_suffix);
        }
        locker.lock();
        flush();
        fclose(m_fd);
        m_fd = fopen(newFile,"a");
        assert(m_fd != nullptr);
    }
    
    //写日志事件
    std::unique_lock<std::mutex> locker(m_mutex);
    m_lineCount++;
    int n = snprintf(m_buff.BeginWrite(), 128,"%d-%02d-%02d %02d:%02d:%02d.%06ld",
                        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, 
                        t->tm_min, t->tm_sec, nowtime.tv_usec);
    m_buff.HasWritten(n);
    __AppendLogLevelTitle(level);

    va_start(va_List,format);
    int m = vsnprintf(m_buff.BeginWrite(),m_buff.WritableBytes(),format,va_List);
    va_end(va_List);

    m_buff.HasWritten(m);
    m_buff.Append("\n\0",2);
    
    if(m_isAsync && m_deque && !m_deque->Full())
    {
        m_deque->push_back(m_buff.RetrieveToStr());
    }
    else
    {
        fputs(m_buff.Peek(),m_fd);
    }
    m_buff.RetrieveAll();
}

void Log::__AppendLogLevelTitle(int level)
{
    switch(level) {
    case 0:
        m_buff.Append("[debug]: ", 9);
        break;
    case 1:
        m_buff.Append("[info] : ", 9);
        break;
    case 2:
        m_buff.Append("[warn] : ", 9);
        break;
    case 3:
        m_buff.Append("[error]: ", 9);
        break;
    default:
        m_buff.Append("[info] : ", 9);
        break;
    }
}

void Log::flush()
{
    if(m_isAsync)
        m_deque->flush();
    fflush(m_fd);
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

