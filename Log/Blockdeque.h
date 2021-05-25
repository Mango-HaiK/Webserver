/**
 * 双端队列
 * */
#ifndef BLOCKDEQUE_H_
#define BLOCKDEQUE_H_

#include <mutex>
#include <deque>
#include <condition_variable>

typedef std::lock_guard<std::mutex> lg_mutex;
typedef std::unique_lock<std::mutex> ul_mutex;

template <typename T>
class Blockdeque
{
public:
    Blockdeque(size_t max_apacity = 1024);
    
    ~Blockdeque();
    
    void Clear();
    bool Empty();
    bool Full();
    void Close();

    size_t size();
    size_t capacity();

    T Front();
    T Back();

    //入队
    void push_back(const T& item);
    void push_front(const T& item);

    //出队
    bool pop(T &item);
    bool pop(T &item,int timeout);  //设置超时

    void flush();

private:
    std::deque<T> m_deque;  
    
    size_t m_capacity;      

    std::mutex m_mutex;

    bool isclose;
    
    std::condition_variable m_Consumer; //消费者
    std::condition_variable m_Porducer; //生产者
};

template<typename T>
Blockdeque<T>::Blockdeque(size_t max_capacity):
                            m_capacity(max_capacity),
                            isclose(false)

{
    assert(m_capacity > 0);
}
template<typename T>
Blockdeque<T>::~Blockdeque()
{
    Close();
}

template<typename T>
void Blockdeque<T>::Close()
{
    {
        lg_mutex locker(m_mutex);
        m_deque.clear();
        isclose = true;
    }
    //唤醒所有等待线程
    m_Consumer.notify_all();
    m_Porducer.notify_all();
}

template<typename T>
bool Blockdeque<T>::Empty()
{
    lg_mutex locker(m_mutex);
    return m_deque.empty();
}

template <typename T>
void Blockdeque<T>::Clear()
{
    lg_mutex locker(m_mutex);
    return m_deque.clear();
}

template <typename T>
bool Blockdeque<T>::Full()
{
    lg_mutex locker(m_mutex);
    return m_deque.size() >= m_capacity;
}

template <typename T>
void Blockdeque<T>::flush()
{
    m_Consumer.notify_one();
}

template <typename T>
T Blockdeque<T>::Front()
{
    lg_mutex locker(m_mutex);
    return m_deque.front();
}

template <typename T>
T Blockdeque<T>::Back()
{
    lg_mutex locker(m_mutex);
    return m_deque.back();
}

template <typename T>
size_t Blockdeque<T>::size()
{
    lg_mutex locker(m_mutex);
    return m_deque.size();
}

template <typename T>
size_t Blockdeque<T>::capacity()
{
    lg_mutex locker(m_mutex);
    return m_capacity;
}

template<typename T>
void Blockdeque<T>::push_back(const T& item)
{
    ul_mutex locker(m_mutex);
    //如果队列超过上限值则阻塞等待
    while (m_deque.size() >= m_capacity)
    {
        m_Porducer.wait(locker);    //此时locker会自动unlock()
    }
    //加入队列
    m_deque.push_back(item);
    //唤醒一个消费者
    m_Consumer.notify_one();
}

template<typename T>
void Blockdeque<T>::push_front(const T &item)
{
    ul_mutex locker(m_mutex);
    while (m_deque.size() >= m_capacity)
    {
        m_Porducer.wait(locker);
    }
    m_deque.push_front(item);
    m_Consumer.notify_one();
}

template<typename T>
bool Blockdeque<T>::pop(T& item)
{
    ul_mutex locker(m_mutex);
    while (m_deque.empty())
    {
        m_Consumer.wait(locker);
        if(isclose) return false;
    }
    //尾进头出
    item = m_deque.front();
    //删除
    m_deque.pop_front();
    //唤醒生产者
    m_Porducer.notify_one();
    return true;
}

template<typename T>
bool Blockdeque<T>::pop(T &item,int timeout)
{
    ul_mutex locker(m_mutex);
    while (m_deque.empty())
    {
        int ret = m_Consumer.wait_for(locker,
                        std::chrono::seconds(timeout)) == std::cv_status::timeout;
        //如果超过等待时间返回
        if (ret || isclose)
            return false;
    }
    m_deque.front(item);
    m_deque.pop_front(item);
    m_Porducer.notify_one();
    return true;
}
#endif 