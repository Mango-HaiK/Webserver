#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 1024

int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        std::cout << "usage: "<< argv[0] <<" ip port_number\n";
        return 1;
    }
    
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    int sock = socket(AF_INET,SOCK_STREAM,0);
    assert(sock >= 0);

    int ret = bind(sock,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(sock,3);
    assert(ret != -1);

    sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int connfd = accept(sock,(sockaddr*)&client,&client_len);
    if(connfd < 0)
    {
        std::cout<<errno;
    }
    else
    {
        char buffer[BUFSIZE];

        memset(&buffer, '\0', BUFSIZE);
        recv(connfd, &buffer, BUFSIZE - 1, 0);
        std::cout<<"got "<< ret <<"byte of normal data "<<buffer<<std::endl;

        //在这输出的带外数据只有c显示，因为紧急标志只能只能带一个字节

        memset(&buffer, '\0', BUFSIZE);
        recv(connfd, &buffer, BUFSIZE - 1, MSG_OOB);
        std::cout<<"got "<< ret <<"byte of oob data "<<buffer<<std::endl;

        memset(&buffer, '\0', BUFSIZE);
        recv(connfd, &buffer, BUFSIZE - 1, 0);
        std::cout<<"got "<< ret <<"byte of normal data "<<buffer<<std::endl;
        close(connfd);
    }
    close(sock);
    return 0;
}