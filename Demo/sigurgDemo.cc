#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#define BUF_SIZE 1024

static int connfd;

//SIGURG信号处理函数
void sig_urg(int sig)
{
    int save_errno = errno;
    char buffer[BUF_SIZE];
    memset(&buffer,'\0',BUF_SIZE);
    int ret = recv(connfd,buffer,BUF_SIZE - 1,MSG_OOB);
    printf("got %d bytes of oob data '%s'\n",ret,buffer);
    errno = save_errno;
}

void addsig(int sig,void (*sig_handler)(int))
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

    sockaddr_in client;
    socklen_t len = sizeof(clilen);
    connfd = accept(listenfd,(sockaddr*)&clilen,&len);
    if(connfd < 0)
    {
        printf("errno is: %d\n",errno);
    }
    else
    {
        addsig(SIGURG,sig_urg);
        //使用SIGURG前设置socket的宿主进程或进程组
        fcntl(connfd,F_SETOWN,getpid());

        char buffer[BUF_SIZE];
        while (1)
        {
            memset(&buffer,'\0',BUF_SIZE);
            ret = recv(connfd,buffer,BUF_SIZE - 1,0);
            if (ret <= 0)
            {
                break;
            }
            printf("got %d bytes of normal data '%s\n",ret,buffer);
        }
        close(connfd);
    }
    close(listenfd);
    return 0;
}