#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

#define USER_LIMIT 5
#define BUFFER_SIZE 1024
#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024
#define PROCESS_LOMIT 1024

//客户端连接结构
struct client_data
{
    sockaddr_in address;
    int connfd;         //连接描述符
    pid_t pid;          //处理这个连接的子进程pid
    int pidfd[2];       //与父进程通信的管道
};

static const char* shm_name = "/my_shm";
int sig_pipefd[2];
int epollfd;
int listenfd;
int shmfd;
char* share_men = 0;
//客户连接组，进程通过该编号取得相关客户连接数据
client_data* user = 0;
//子进程和客户连接的映射关系表，使用pid索引该数组取得所处理的客户连接编号
int* sub_process = 0;
//当前客户数量
int user_count = 0;
bool stop_child = false;

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
    event.data.fd = fd;
    event.events = EPOLLET | EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}

void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(sig_pipefd[1],(char*)&msg,1,0);
    errno = save_errno;
}
void addsig(int sig,void (*handle)(int),bool restart = true)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = handle;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL) != -1);
}
int main(int argc, char const *argv[])
{
    if (argc <= 2)
    {
        printf("usage:%s ip port",argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int listenfd;

    sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd >= 0);

    int ret = bind(listenfd,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret != -1);

    user_count = 0;
    user = new client_data[USER_LIMIT + 1];
    sub_process = new int[PROCESS_LOMIT];
    for (int i = 0; i < PROCESS_LOMIT; i++)
    {
        sub_process[i] = -1;
    }
    
    epollfd = epoll_create(5);
    epoll_event events[MAX_EVENT_NUMBER];
    assert(epollfd != -1);
    addfd(epollfd,listenfd);

    ret = socketpair(AF_UNIX,SOCK_STREAM,0,sig_pipefd);
    assert(ret != -1);
    setnonblocking(sig_pipefd[1]);
    addfd(epollfd,sig_pipefd[0]);

    addsig(SIGCHLD,sig_handler);
    addsig(SIGTERM,sig_handler);
    addsig(SIGINT,sig_handler);
    addsig(SIGPIPE,SIG_IGN);
    bool stop_server = false;
    bool terminate = false;

    //创建共享内存，作为所有客户socket连接的读缓存
    shmfd = open(shm_name,O_CREAT | O_RDWR,0666);
    assert(shmfd != -1);

    share_men = (char*)mmap(NULL,USER_LIMIT * BUFFER_SIZE,PROT_READ |
                                PROT_WRITE,MAP_SHARED,shmfd,0);
    assert(share_men != MAP_FAILED);
    close(shmfd);


    return 0;
}
