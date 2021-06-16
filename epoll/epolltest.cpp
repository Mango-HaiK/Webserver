#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#include "epoller.h"

int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage:%s ip port\n",argv[0]);
        return 1;
    }
    int n;
    
    char const* ip = argv[1];
    int port = atoi(argv[2]);

    int ret,listenfd,connfd;
    sockaddr_in address,client_address;
    socklen_t client_len;

    address = {0};
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd != -1);

    ret = bind(listenfd,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret != -1);

    setnonblocking(listenfd);
    
    Epoller epoller;
    uint32_t event = EPOLLIN | EPOLLOUT;
    epoller.AddFd(listenfd, event);

    bool isclose = false;
    while (!isclose)
    {
        int cnt = epoller.Wait(-1);
        for (int i = 0; i < cnt; i++)
        {
            printf("2");
            if (listenfd == epoller.GetEventFd(i))
            {
                printf("hello");
            }
        }
    }

    return 0;
}
