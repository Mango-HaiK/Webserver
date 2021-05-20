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
#define PROCESS_LOMIT 65535

//客户端连接结构
struct client_data
{
    sockaddr_in address;
    int connfd;         //连接描述符
    pid_t pid;          //处理这个连接的子进程pid
    int pipefd[2];       //与父进程通信的管道
};

static const char* shm_name = "/my_shm";
int sig_pipefd[2];
int epollfd;
int listenfd;
int shmfd;
char* share_men = 0;
//客户连接组，进程通过该编号取得相关客户连接数据
client_data* users = 0;
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

void del_resource()
{
    close(sig_pipefd[0]);
    close(sig_pipefd[1]);
    close(listenfd);
    close(epollfd);
    shm_unlink(shm_name);
    delete[] users;
    delete[] sub_process;
}

void child_term_handler(int sig)
{
    stop_child = true;
}
//子进程处理函数，参数idx指出该子进程处理的客户连接编号，user是保存所有客户连接数据的数组
//share_men为共享内存起始地址。
int run_child(int idx,client_data* users, char* share_men)
{
    epoll_event events[MAX_EVENT_NUMBER];
    //子进程使用I/O复用同时监听客户连接socket和父进程通信的管道
    int child_epollfd = epoll_create(5);
    assert(child_epollfd != -1);
    int connfd = users[idx].connfd;
    addfd(child_epollfd,connfd);
    int pipefd = users[idx].pipefd[1];
    addfd(child_epollfd,pipefd);

    int ret;

    //子进程自己的信号处理函数
    addsig(SIGTERM,child_term_handler,false);
    while (!stop_child)
    {
        int number = epoll_wait(child_epollfd,events,MAX_EVENT_NUMBER,-1);
        if ((number < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }
        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            if ((sockfd == connfd) && (events[i].events & EPOLLIN))
            {
                memset(share_men + idx * BUFFER_SIZE,'\0',BUFFER_SIZE);
                ret = recv(connfd,share_men + idx * BUFFER_SIZE, BUFFER_SIZE - 1,0);
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        stop_child = true;
                    }
                }
                else if (ret == 0)
                {
                    stop_child = true;
                }
                else
                {
                    //读取到客户端数据通知主进程来处理
                    send(pipefd,(char*)&idx,sizeof(idx),0);
                }
            }
            //主进程通知本进程将第client个客户的数据发送到本进程负责的客户端
            else if ((sockfd == pipefd) && (events[i].events & EPOLLIN))
            {
                int client = 0;
                //接受主进程发送来的数据,即有客户数据到达的连接编号
                ret = recv(sockfd,(char*)&client,sizeof(client),0);
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        stop_child = true;
                    }
                }
                else if (ret == 0)
                {
                    stop_child = true;
                }
                else
                {
                    send(connfd,share_men + client * BUFFER_SIZE,BUFFER_SIZE,0);
                }
            }
            else
                continue;
        }  
    }
    close(connfd);
    close(pipefd);
    close(child_epollfd);
    return 0;
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
    users = new client_data[USER_LIMIT + 1];
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
    shmfd = shm_open(shm_name,O_CREAT | O_RDWR,0666);
    assert(shmfd != -1);

    share_men = (char*)mmap(NULL,USER_LIMIT * BUFFER_SIZE,PROT_READ |
                                PROT_WRITE,MAP_SHARED,shmfd,0);
    assert(share_men != MAP_FAILED);
    close(shmfd);

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
                socklen_t client_len = sizeof(client_address);
                int connfd = accept(listenfd,(sockaddr*)&client_address,&client_len);
                
                if (connfd < 0)
                {
                    printf("errno is %d\n",errno);
                    continue;
                }
                if (user_count > USER_LIMIT)
                {
                    const char* info = "too many user\n";
                    printf("%s",info);
                    send(connfd,info,strlen(info),0);
                    continue;
                }
                //保存第user_count个客户连接的相关数据
                users[user_count].address = client_address;
                users[user_count].connfd = connfd;
                //主进程和子进程建立管道通信
                ret = socketpair(AF_UNIX,SOCK_STREAM,0,users[user_count].pipefd);
                assert(ret != -1);
                pid_t pid = fork();
                if (pid < 0)
                {
                    close(connfd);
                    continue;
                }
                else if (pid == 0)
                {
                    close(epollfd);
                    close(listenfd);
                    close(users[user_count].pipefd[0]);
                    close(sig_pipefd[0]);
                    close(sig_pipefd[1]);
                    run_child(user_count,users,share_men);
                    munmap((void*)share_men,USER_LIMIT * BUFFER_SIZE);
                    exit(0);
                }
                else{
                    close(connfd);
                    close(users[user_count].pipefd[1]);
                    addfd(epollfd,users[user_count].pipefd[0]);
                    users[user_count].pid = pid;
                    //记录新的客户连接在数组users中的索引值，建立进程pid和该索引值之间的映射关系
                    sub_process[pid] = user_count;
                    user_count++;
                }
            }
            //处理信号事件
            else if ((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv(sig_pipefd[0],signals,sizeof(signals),0);
                if (ret == -1)
                {
                    continue;
                }else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    for (int i = 0;i < ret;i++)
                    {
                        switch (signals[i])
                        {
                            //子进程退出，表示有某个客户关闭了连接
                        case SIGCHLD:
                            pid_t pid;
                            int stat;
                            while ((pid = waitpid(-1,&stat,WNOHANG)) > 0)
                            {
                                //用子进程的pid取得被关闭的客户连接的编号
                                int del_user = sub_process[pid];
                                sub_process[pid] = -1;
                                if ((del_user < 0) || (del_user > USER_LIMIT))
                                {
                                    continue;
                                }
                                //清除第del_user个客户连接使用的相关数据
                                epoll_ctl(epollfd,EPOLL_CTL_DEL,users[del_user].pipefd[0],events);
                                close(users[del_user].pipefd[0]);
                                users[del_user] = users[--user_count];
                                sub_process[users[del_user].pid] = del_user;
                            }
                            if (terminate && user_count == 0)
                            {
                                stop_server = true;
                            }
                            break;
                        case SIGTERM:
                        case SIGINT:
                        {
                            //结束服务器程序
                            printf("kill all thr child now\n");
                            if (user_count == 0)
                            {
                                stop_server = true;
                                break;
                            }
                            for (int i = 0; i < user_count; i++)
                            {
                                int pid = users[i].pid;
                                kill(pid,SIGTERM);
                            }
                            terminate = true;
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                int child = 0;
                //读管道数据，child记录是哪个客户的连接有数据到达
                ret = recv(sockfd,(char*)&child,sizeof(child),0);
                printf("read data from child accross pope\n");
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
                    //向除负责第child个客户连接的子进程之外的其他子进程发送消息
                    //通知有客户数据要写。
                    for (int j = 0; j < user_count; j++)
                    {
                        if (users[j].pipefd[0] != sockfd)
                        {
                            printf("send data to child accross pipe\n");
                            send(users[j].pipefd[0],(char *)&child,sizeof(child),0);
                        }
                    }
                }
            }
        }
    }
    
    del_resource();
    return 0;
}
