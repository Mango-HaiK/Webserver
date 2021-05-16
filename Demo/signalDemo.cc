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
#include <signal.h>
#include <pthread.h>

#define MAX_EVENT_NUMBER 1024
static int pipefd[2];

int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}

//信号处理函数
void sig_handler(int sig)
{
    //保留原来的errno,在函数后恢复，以保证函数的可重入性
    int save_errno = errno;
    int msg = sig;
    //将信号写入管道,以通知主循环
    send(pipefd[1],(char*)&msg,1,0);
    errno = save_errno;
}

//设置信号处理函数
void addsig(int sig)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL) != -1);
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
    addfd(epollfd,listenfd);

    //创建管道，注册pipefd上的可读事件
    ret = socketpair(AF_UNIX,SOCK_DGRAM,0,pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd,pipefd[0]);
    
    //设置信号处理的函数
    addsig(SIGHUP);
    addsig(SIGCHLD);
    addsig(SIGTERM);
    addsig(SIGINT);
    bool stop_server = false;

    while (!stop_server)
    {
        int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if ((number < 0) && (errno != EINTR))
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
                socklen_t len;
                len = sizeof(client_address);

                int connfd = accept(listenfd,(sockaddr*)&client_address,&len);
                addfd(epollfd,connfd);
            }
            else if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv(pipefd[0],signals,sizeof(signals),0);
                if (ret == -1)
                {
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    //每个信号占一个字节,所以按字节来逐个处理
                    for (int i = 0; i < ret; i++)
                    {
                        switch (signals[i])
                        {
                        case SIGCHLD:
                        case SIGHUP:
                            continue;
                        case SIGTERM:
                        case SIGINT:
                            stop_server = true;
                        default:
                            break;
                        }
                    }
                    
                }
                
            }
        }
        
        
    }
    printf("close fds\n");
    close(pipefd[0]);
    close(pipefd[1]);
    close(listenfd);
    return 0;
}
