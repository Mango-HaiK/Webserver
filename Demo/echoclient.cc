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

#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage:%s ip port",argv[0]);
        return 1;
    }

    char const* ip = argv[1];
    int port = atoi(argv[2]);

    int ret, adddress,sock;
    sockaddr_in address;
    socklen_t address_len;
    
    bzero(&address,sizeof(address));
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    sock = socket(AF_INET,SOCK_STREAM,0);
    address_len = sizeof(address);
    ret = connect(sock,(sockaddr*)&address,address_len);
    if (ret < 0)
    {
        printf("connection failed\n");
        close(sock);
        return 1;
    }
    
    pollfd fds[2];
    //注册文件描述符0（标准输入），和sock的可读事件
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sock;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;

    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    ret = pipe(pipefd);
    assert(ret != -1);

    while (1)
    {
        ret = poll(fds,2,-1);
        if(ret < 0)
        {
            printf("poll failure\n");
            break;
        }
        if (fds[0].revents & POLLRDHUP)
        {
            printf("server close the connection\n");
            break;
        }else if (fds[1].revents & POLLIN)
        {
            memset(read_buf,'\0',BUFFER_SIZE);
            recv(sock,read_buf,BUFFER_SIZE - 1,0);
            printf("%s\n", read_buf);
        }
        
        if(fds[0].revents & POLLIN)
        {
            ret = splice(0, NULL,pipefd[1],NULL,32768,
                    SPLICE_F_MORE | SPLICE_F_MOVE);
            ret = splice(pipefd[0],NULL,sock,NULL,32768,
                    SPLICE_F_MORE | SPLICE_F_MOVE);
        }
    }
    close(sock);
    return 0;
}
