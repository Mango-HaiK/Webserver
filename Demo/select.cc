#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage:%s [ip] [port]",argv[0]);
        return 1;
    }
    char const* ip = argv[1];
    int port = atoi(argv[2]);

    int listenfd,connfd,ret;
    sockaddr_in address,clilent_address;
    socklen_t clilen;

    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd >= -1);

    ret = bind(listenfd,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret != -1);

    clilen = sizeof(clilent_address);
    connfd = accept(listenfd,(sockaddr*)&clilent_address,&clilen);

    if(connfd < 0)
    {
        printf("errno is: %d\n",errno);
        close(listenfd);
    }

    char buf[1024];
    fd_set read_fds;
    fd_set exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&exception_fds);

    while (1)
    {
        memset(buf,'\0',sizeof(buf));

        FD_SET(connfd,&read_fds);
        FD_SET(connfd,&exception_fds);
        ret = select(connfd + 1,&read_fds,NULL,&exception_fds,NULL);
        if(ret < 0)
        {
            printf("selection failure.\n");
            break;
        }

        //对于可读事件，使用recv 函数读取数据
        if (FD_ISSET(connfd,&read_fds))
        {
            ret = recv(connfd,buf,sizeof(buf) - 1,0);
            if(ret <= 0) break;
            printf("get %d bytes of normal data: %s\n",ret,buf);
        }
        else if (FD_ISSET(connfd,&exception_fds))
        {
            ret = recv(connfd,buf,sizeof(buf) - 1,MSG_OOB);
            if(ret <= 0) break;
            printf("get %d bytes of oob data: %s\n",ret,buf);
        }
    }
    close(connfd);
    close(listenfd);
    return 0;
}

