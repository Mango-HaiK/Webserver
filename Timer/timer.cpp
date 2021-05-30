#include "timer.h"

HeapTimer::HeapTimer()
{
    m_heap.reserve(64);
}

void HeapTimer::__swapNode(size_t i,size_t j)
{
    assert((i < m_heap.size() && i >= 0) ||
         (j < m_heap.size() && j >= 0));
    std::swap(m_heap[i],m_heap[j]);
    m_ref[m_heap[i].id] = i;
    m_ref[m_heap[j].id] = j;
}

void HeapTimer::__shiftup(size_t i)
{
    assert(i < m_heap.size() && i > 0);
    size_t j = (i - 1) >> 1;        //父节点 
    while (j >= 0)
    {
        //满足小堆
        if (m_heap[i] < m_heap[j])
        {
            break;
        }
        __swapNode(i,j);
        //继续向上操作父节点
        i = j;
        j = (i - 1) >> 1;
    }
}

bool HeapTimer::__shiftdown(size_t index,size_t n)
{
    assert(index >= 0 && index < m_heap.size());
    assert(n >= 0 && m_heap.size() >= n);
    size_t i = index;
    size_t j = index << 1 + 1;
    //存在该子节点
    while (j < n)
    {
        //与两个节点中最小的子节点交换
        if (j + 1 < n && m_heap[j + 1] < m_heap[j])
        {
            j++;
        }
        //满足小堆
        if (m_heap[i] < m_heap[j])
        {
            break;
        }
        __swapNode(i,j);
        i = j;
        j = (i << 1) + 1;
    }
    return i < j;
}

void HeapTimer::add(int id,int timeout,const TimeCallBackFunc& cbfunc)
{
    assert(id >= 0);
    size_t i;
    //不存在该节点，该节点为新节点
    if(m_ref.count(id) == 0)
    {
        //堆尾插入
        i = m_heap.size();
        m_ref[id] = i;
        m_heap.push_back({id, Clock::now() + MS(timeout), cbfunc});
        __shiftup(i);
    }else   //堆中已经存在该节点，调整
    {
        i = m_ref[id];
        m_heap[i].expires = Clock::now() + MS(timeout);
        m_heap[i].cbFunc = cbfunc;
        if (!__shiftdown(i, m_heap.size()))
        {
            __shiftup(i);
        }
    }
}

void HeapTimer::__del(size_t index )
{
    assert(!m_heap.empty() && index >= 0 && index < m_heap.size());
    //将要删除的元素置于堆尾
    size_t i = index;
    size_t n = m_heap.size() - 1;
    assert(i <= n);
    if (i < n)
    {
        __swapNode(i, n);
        if (!__shiftdown(i, n))
        {
            __shiftup(i);
        }
    }
    //删除元素
    m_ref.erase(m_heap.back().id);
    m_heap.pop_back();
}

void HeapTimer::doWork(int id)
{
    if (m_heap.empty() || m_ref.count(id) == 0)
    {
        return;
    }
    size_t i = m_ref[id];
    //为什么需要临时node？
    TimerNode node = m_heap[i];
    node.cbFunc();
    __del(i);
}

void HeapTimer::adjust(int id,int timeout)
{
    assert(!m_heap.empty() && m_ref.count(id) > 0);
    m_heap[m_ref[id]].expires = Clock::now() + MS(timeout);
    __shiftdown(m_ref[id], m_heap.size());
}

void HeapTimer::pop()
{
    assert(!m_heap.empty());
    __del(0);
}

void HeapTimer::clear()
{
    m_heap.clear();
    m_ref.clear();
}

void HeapTimer::tick()
{
    if (m_heap.empty())
    {
        return;
    }
    while (!m_heap.empty())
    {
        TimerNode node = m_heap.front();
        //判断到达超时时间
        if (std::chrono::duration_cast<MS>
                    (node.expires - Clock::now()).count() > 0)
        {
            break;
        }
        node.cbFunc();
        pop();
    }
}

int HeapTimer::getNextTick()
{
    tick();
    int res = -1;
    if (!m_heap.empty())
    {
        res = std::chrono::duration_cast<MS>
                    (m_heap.front().expires - Clock::now()).count();
        if (res < 0)
        {
            res = 0;
        }
    }
    return res;
}