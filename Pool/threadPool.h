#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>


class ThreadPool
{
public:

    explicit ThreadPool(size_t threadSize = 8) : m_pool(std::make_shared<Pool>()){
        //开启线程后线程循环检测任务队列中的任务
        for (size_t i = 0; i < threadSize; i++)
        {
            std::thread(
                [this](){
                    std::unique_lock<std::mutex> locker(m_pool->mutex);
                    while (true)
                    {
                        if (!m_pool->tasks.empty())
                        {
                            auto task = std::move(m_pool->tasks.front());
                            m_pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        }else if(m_pool->isClose)
                        {
                            break;
                        }
                        else{
                            m_pool->cv.wait(locker);
                        }
                    }
                }
            ).detach();
        }
    }

    ~ThreadPool()
    {
        //m_pool肯定是存在的，但是未必有数据
        if (static_cast<bool> (m_pool))
        {
            {
                std::lock_guard<std::mutex> locker(m_pool->mutex);
                m_pool->isClose = false;
            }
            m_pool->cv.notify_all();
        }
    }

    template<typename F>
    void AddTask(F&& task)
    {
        {
        std::lock_guard<std::mutex> locker(m_pool->mutex);
        m_pool->tasks.emplace(std::forward<F>(task));
        }
        m_pool->cv.notify_one();
    }
private:
    
    struct Pool
    {
        std::mutex mutex;

        //函数类型可以再改进一下
        std::queue<std::function<void()> > tasks;     //任务队列

        std::condition_variable cv;                 
        
        bool isClose;
    };

    std::shared_ptr<Pool> m_pool;
};


#endif