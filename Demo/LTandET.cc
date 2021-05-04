#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> //inet_pton
#include <stdio.h>
#include <string.h>
#include <stdlib.h>     //atoi
#include <assert.h>     //assert
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10

//将文件描述符设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

//将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件中
void addfd(int epollfd,int fd,bool enable_et)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et)
    {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}
//LT工作模式
void lt(epoll_event* events,int number,int epollfd,int listenfd)
{
    char buf[BUFFER_SIZE];
    for (int i = 0; i < number; i++)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            sockaddr_in client_address;
            socklen_t client_len = sizeof(client_address);
            int connfd = accept(listenfd,(sockaddr*)&client_address,&client_len);
            addfd(epollfd,connfd,false);
        }else if (events[i].events &EPOLLIN)
        {
            printf("event trigger once\n");
            memset(&buf,'\0',BUFFER_SIZE);
            int ret = recv(sockfd,buf,BUFFER_SIZE - 1,0);
            if(ret <= 0)
            {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content: %s\n",ret,buf);
        }else
        {
            printf("something other happened \n");
        }
    }
}

//ET工作模式
void et(epoll_event* events,int number, int epollfd,int listenfd)
{
    char buf[BUFFER_SIZE];
    for (int i = 0; i < number; i++)
    {
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd)
        {
            sockaddr_in client_address;
            socklen_t client_len = sizeof(client_address);
            int connfd = accept(sockfd,(sockaddr*)&client_address,&client_len);
            addfd(epollfd,connfd,true);
        }else if (events[i].events & EPOLLIN)
        {
            printf("event trigger once\n");
            while (1)
            {
                memset(&buf,'\0',BUFFER_SIZE);
                int ret = recv(sockfd,buf,BUFFER_SIZE - 1,0);
                if(ret < 0)
                {
                    if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                }else if(ret == 0)
                {
                    close(sockfd);
                }
                else{
                    printf("get %d bytes of content: %s\n",ret,buf);
                }
            }
        }
        else
        {
            printf("something other happened \n");
        }
        
    }
    
}
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage:%s ip port\n",argv[0]);
        return 1;
    }
    int n;
    scanf("%d",&n);
    
    char const* ip = argv[1];
    int port = atoi(argv[2]);

    int ret,listenfd,connfd;
    sockaddr_in address,client_address;
    socklen_t client_len;

    bzero(&address,sizeof(address));
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd != -1);

    ret = bind(listenfd,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    
    addfd(epollfd,listenfd,true);

    while (1)
    {
        ret = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if(ret < 0)
        {
            printf("epoll failure\n");
            break;
        }
        if(n == 1) lt(events,ret,epollfd,listenfd);
        else et(events,ret,epollfd,listenfd);
    }
    close(listenfd);

    return 0;
}
