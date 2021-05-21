/**
 * 日志
 * */
#ifndef LOG_H_
#define LOG_H_

#include <mutex>
#include "../Buffer/buffer.h"

class Log
{
private:
    Log();  //单例私有化构造函数
    

public:
    void Init();
};

#endif