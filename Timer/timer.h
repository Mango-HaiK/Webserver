#ifndef TIMER_H_
#define TIMER_H_

#include <functional>   //function
#include <chrono>
#include <vector>
#include <time.h>
#include <assert.h>
#include <unordered_map>

typedef std::function<void()> TimeCallBackFunc;
typedef std::chrono::high_resolution_clock Clock;
typedef Clock::time_point TimeStamp;
typedef std::chrono::milliseconds MS;

/*定时器*/
struct TimerNode
{
    int id;
    TimeStamp  expires;         //有效期
    TimeCallBackFunc cbFunc;    //回调函数
    /*TimeStampk对时间点进行算术计算*/
    bool operator<(TimerNode& t)
    {
        return expires < t.expires;
    }
};


class HeapTimer
{
public:
    HeapTimer();
    
    //最小有效期定时器到期，执行心搏函数处理超时结点
    void tick();

    //添加定时器-设置超时时间和超时后的所需要执行的回调函数
    void add(int id, int timeout, const TimeCallBackFunc& cbfunc);

    //删除堆中指定id的节点
    void doWork(int id);

    //调整指定id的节点
    void adjust(int id, int newExpires);

    int getNextTick();

    void pop();

    void clear();

private:

    //在堆中删除指定位置的结点
    void __del(size_t index);

    //向上调整
    void __shiftup(size_t id);

    //向下调整
    bool __shiftdown(size_t index,size_t n);

    //交换结点
    void __swapNode(size_t,size_t);

    std::vector<TimerNode> m_heap;  //存放定时器结点的堆
    
    std::unordered_map<int ,size_t> m_ref;
};


#endif