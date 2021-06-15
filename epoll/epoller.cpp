#include "epoller.h"

Epoller::Epoller(int maxEvent) : m_epollfd(epoll_create(1024))
{
    assert(m_epollfd >= 0);
}

Epoller::~Epoller()
{
    close(m_epollfd);
}

bool Epoller::AddFd(int fd,uint32_t _event)
{
    if(fd < 0) return false;
    epoll_event event =  {0};
    event.events = _event;
    event.data.fd = fd;
    return 0 == epoll_ctl(m_epollfd,EPOLL_CTL_ADD,fd,&event);
}

bool Epoller::ModFd(int fd, uint32_t _event)
{
    if(fd < 0) return false;
    epoll_event event = {0};
    event.events = _event;
    event.data.fd = fd;
    return 0 == epoll_ctl(m_epollfd,EPOLL_CTL_MOD,fd, &event);
}

bool Epoller::DelFd(int fd)
{
    if(fd < 0) return false;
    epoll_event event = {0};
    return 0 == epoll_ctl(m_epollfd,EPOLL_CTL_DEL,fd,&event);
}

int Epoller::Wait(int timeout)
{
   return epoll_wait(m_epollfd,&m_events[0],m_events.size(),timeout); 
}

epoll_event Epoller::GetEpollEvent(size_t i) const
{
    return m_events[i];
}

int Epoller::GetEventFd(size_t i) const
{
    return m_events[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const
{
    return m_events[i].events;
}