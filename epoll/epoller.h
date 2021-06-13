#ifndef EPOLLER_H_
#define EPOLLER_H_

#include <sys/epoll.h>
#include <vector>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

class Epoller
{
public:
    explicit Epoller(int maxEvent = 1024);

    ~Epoller();

    bool AddFd(int fd,uint32_t _event);

    bool ModFd(int fd,uint32_t _event);

    bool DelFd(int fd);

    /*返回事件数目，可设置等待时间默认阻塞等待*/
    int Wait(int timeout = -1);

    /*返回待处理事件链表的第i个fd*/
    int GetEventFd(size_t i) const;

    /*返回第i个fd的事件*/
    uint32_t GetEvents(size_t i) const;

    epoll_event GetEpollEvent(size_t i) const;
private:
    int m_epollfd;

    std::vector<epoll_event> m_events;

};

#endif  //EPOLLER_H_