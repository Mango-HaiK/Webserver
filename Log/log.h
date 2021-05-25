/**
 * 日志
 * */
#ifndef LOG_H_
#define LOG_H_

#include <mutex>
#include <string>
#include <thread>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "../Buffer/buffer.h"
#include "Blockdeque.h"

class Log
{
private:
    Log();  //单例私有化构造函数
    ~Log();
    void __AppendLogLevelTitle(int level);
    void __AsyncWrite();

    static const int LOG_PATH_LEN = 256;   //日志路径长度
    static const int LOG_NAME_LEN = 256;    //文件名长度    
    static const int MAX_LINES = 50000;      //最大行数

    const char* m_path;
    const char* m_suffix;

    int m_lineCount;    //行数
    int m_toDay;        

    bool m_isOpen;

    int m_level;
    bool m_isAsync;

    Buffer m_buff;
    FILE *m_fd;
    std::unique_ptr<Blockdeque<std::string> > m_deque;   //用于写的队列
    std::unique_ptr<std::thread> m_writeThread;         //写线程
    std::mutex m_mutex;

public:
    //初始化
    void Init(int level,const char* path = "./log",
                const char* suffix = ".log",
                    size_t maxDequeSize = 1024);

    //返回单实例对象
    static Log* Instance();
    //异步写线程
    static void FlushLogThread();

    void write(int level,const char* format,...);
    void flush();

    void SetLevel(int level);
    int GetLevel();
    bool IsOpen() {return m_isOpen;};
};

#define LOG_BASE(level, format, ...)\
        do {\
            Log* log = Log::Instance();\
            if (log->IsOpen() && log->GetLevel() <= level)\
            {\
                log->write(level, format, ##__VA_ARGS__); \
                log->flush();\
            }\
        } while(0);

#define LOG_DEBUG(format, ...) do{ LOG_BASE(0, format, ##__VA_ARGS__) }while(0);
#define LOG_INFO(format, ...) do{ LOG_BASE(1, format, ##__VA_ARGS__) }while(0);
#define LOG_WARN(format, ...) do{ LOG_BASE(2, format, ##__VA_ARGS__) }while(0);
#define LOG_ERROR(format, ...) do{ LOG_BASE(3, format, ##__VA_ARGS__) }while(0);
#endif