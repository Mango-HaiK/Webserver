#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#define MAX_EVENT_NUMBER 1024
#define TCP_BUFFER_SIZE 512
#define UDP_BUFFER_SIZE 1024

int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void addfd(int epollfd,int fd)
{
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}

int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage:%s ip port",argv[0]);
        return 1;
    }

    char const* ip = argv[1];
    int port = atoi(argv[2]);

    int ret, adddress,listenfd,connfd;
    sockaddr_in address;
    socklen_t address_len;
    
    bzero(&address,sizeof(address));
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    
    //创建TCP socket，并将其绑定到端口port上
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd >= 0);

    ret = bind(listenfd,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret != -1);

    //创建UDP socket，
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);
    
    int udpfd = socket(AF_INET,SOCK_DGRAM,0);
    assert(udpfd >= 0);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);

    //注册TCP socket和UDP socket上的可读事件
    addfd(epollfd,listenfd);
    addfd(epollfd,udpfd);

    while (1)
    {
        int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if (number < 0)
        {
            printf("epoll failure\n");
            break;
        }
        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd)
            {
                sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd,(sockaddr*)&client_address,&client_addrlength);
                addfd(epollfd,connfd);
            }
            else if(sockfd == udpfd)
            {
                char buf[UDP_BUFFER_SIZE];
                memset(buf,'\0',UDP_BUFFER_SIZE);
                sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                ret = recvfrom(udpfd,buf,UDP_BUFFER_SIZE - 1, 0,
                                (sockaddr*)&client_address,&client_addrlength);
                if (ret > 0)
                {
                    sendto(udpfd,buf,UDP_BUFFER_SIZE - 1, 0,
                                (sockaddr*)&client_address,client_addrlength);
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                char buf[TCP_BUFFER_SIZE];
                while (1)
                {
                    memset(&buf,'\0',TCP_BUFFER_SIZE);
                    ret = recv(sockfd,buf,TCP_BUFFER_SIZE - 1, 0);
                    if (ret < 0)
                    {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                        {
                            break;
                        }
                        close(sockfd);
                        break;
                    }
                    else if(ret == 0)
                    {
                        close(sockfd);
                    }
                    else
                    {
                        send(sockfd,buf,ret,0);
                    }
                }
            }
            else
            {
                printf("something else happened \n");
            }
        }
    }
    close(listenfd);
    return 0;
}
