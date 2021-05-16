#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>     //atoi
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//time 为超时时间
int unblock_connect(const char *ip,int port,int time)
{
    int ret = 0;
    sockaddr_in address;
    bzero(&address,sizeof(address));
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    int fdopt = setnonblocking(sockfd);
    ret = connect(sockfd, (sockaddr*)&address,sizeof(address));
    if(ret == 0)
    {
        printf("connect with server immediately\n");
        fcntl(sockfd,F_SETFL,fdopt);
        return sockfd;
    }
    else if(errno != EINPROGRESS)
    {
        //连接未立即建立，
        printf("unblock connect not support\n");
        return -1;
    }
    fd_set readfds;
    fd_set writefds;
    timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd,&writefds);

    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    ret = select(sockfd,NULL,&writefds,NULL,&timeout);
    if(ret <= 0)
    {
        printf("connection time out\n");
        close(sockfd);
        return -1;
    }
    if (! FD_ISSET(sockfd,&writefds))
    {
        printf("no events on sockfd found\n");
        close(sockfd);
        return -1;
    }

    int error = 0;
    socklen_t len =sizeof(error);

    //调用getsockopt来获取并清除sockfd上的错误
    if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len))
    {
        printf("get socket option failed\n");
        close(sockfd);
        return -1;
    }

    //错误号不为0表示连接出错
    if(error != 0)
    {
        printf("connection failed after select with the error:%d\n",error);
        close(sockfd);
        return -1;
    }
    //连接成功
    printf("connection ready after select with the socket:%d\n",sockfd);
    fcntl(sockfd,F_SETFL,fdopt);
    return sockfd;
}

int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip port\n",argv[0]);
        return 1;
    }

    char const *ip = argv[1];
    int port = atoi(argv[2]);
    
    int sockfd = unblock_connect(ip,port,10);
    if(sockfd < 0)
        return 1;
    close(sockfd);

    return 0;
}
