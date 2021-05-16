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
#define BUFFER_SIZE 1024
#define USER_LIMIT 5        //最大用户数量
#define FD_LIMIT 65535      //文件描述符数量限制
struct client_data
{
    sockaddr_in address;
    char* write_buf;
    char buf[BUFFER_SIZE];
};

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

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd >= 0);

    ret = bind(listenfd,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret != -1);

    client_data* users = new client_data[FD_LIMIT];

    pollfd fds[USER_LIMIT + 1];
    int user_counter = 0;
    for (int i = 1; i <= USER_LIMIT; i++)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }

    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    while (1)
    {
        ret = poll(fds, user_counter + 1,-1);
        if (ret < 0)
        {
            printf("poll failure\n");
            break;
        }
        for (int i = 0; i < user_counter + 1; i++)
        {
            if((fds[i].fd == listenfd) && (fds[i].revents & POLLIN))
            {
                sockaddr_in client_address;
                socklen_t client_len = sizeof(client_address);
                int connfd = accept(listenfd,(sockaddr*)&address,&client_len);
                if(connfd < 0)
                {
                    printf("errno is :%d \n",errno);
                    continue;
                }
                if(user_counter >= USER_LIMIT)
                {
                    const char* info = "too many users\n";
                    printf("%s",info);
                    send(connfd,info,strlen(info),0);
                    close(connfd);
                    continue;
                }
                //对于新的连接，同时修改fds和user数组
                user_counter++;
                users[connfd].address = client_address;
                setnonblocking(connfd);
                fds[user_counter].fd = connfd;
                fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_counter].revents = 0;
                printf("comes a new user,now have %d user\n",user_counter);
            }
            else if (fds[i].revents & POLLERR)
            {
                printf("get an error from %d",fds[i].fd);
                char errors[100];
                memset(errors,'\0',100);
                socklen_t len = sizeof(errors);
                if (getsockopt(fds[i].fd, SOL_SOCKET,SO_ERROR,&errors,&len) < 0)
                {
                    printf("get sockte option failed\n");
                }
                continue;
            }
            else if(fds[i].revents & POLLRDHUP)
            {
                //客户关闭连接，服务器也关闭
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i] = fds[user_counter];
                i--;
                user_counter--;
                printf("a client left\n");
            }
            else if(fds[i].revents & POLLIN)
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf,'\0',BUFFER_SIZE);
                ret = recv(connfd,users[connfd].buf,BUFFER_SIZE - 1, 0);
                printf("get %d bytes of client data %s from %d\n",ret,users[connfd].buf,connfd);
                if (ret < 0)
                {
                    //读出错关闭连接
                    if(errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counter].fd];
                        fds[i] = fds[user_counter];
                        i--;
                        user_counter--;
                    }
                }
                else if(ret == 0)
                {}
                else
                {
                    //接收到数据，通知其他socket连接准备写数据
                    for (int j = 1; j <= user_counter; j++)
                    {
                        if(fds[j].fd == connfd)
                        {
                            continue;
                        }
                        fds[j].events |= ~POLLIN;
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].buf;
                    }
                }
            }
            else if (fds[i].revents & POLLOUT)
            {
                int connfd = fds[i].fd;
                if (! users[connfd].write_buf)
                {
                    continue;
                }
                ret = send(connfd,users[connfd].write_buf,strlen(users[connfd].write_buf),0);
                users[connfd].write_buf = NULL;
                //写完数据重新注册fds[i]上的可读事件
                fds[i].events |= ~POLLOUT;
                fds[i].events |= POLLIN;
                }
        }
    }
    delete[] users;
    close(listenfd);
    return 0;
}
