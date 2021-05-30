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
    
}
