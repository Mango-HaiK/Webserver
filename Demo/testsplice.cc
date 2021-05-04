#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if(argc <= 2)
    {
        std::cout<<"usage: "<<argv[0]<<" ip port"<<std::endl;
        return 1;
    }

    int sock = socket(AF_INET,SOCK_STREAM,0);

    char *ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    int ret = bind(sock,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(sock,5);
    assert(ret != -1);

    sockaddr_in client_address;
    bzero(&client_address,sizeof(client_address));
    socklen_t clilen = sizeof(client_address);
    int connfd = accept(sock,(sockaddr*)&client_address,&clilen);

    if(connfd < 0)
    {
        std::cout<< " errno\n";
    }
    else{

        std::cout<<"listen...\n";
        int pipefd[2];

        //创建管道
        ret = pipe(pipefd);
        assert(ret != -1);

        //向管道传入客户数据
        ret = splice(connfd,NULL,pipefd[1],NULL,32768,
                    SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);

        //将管道数据传入客户sock
        ret = splice(pipefd[0],NULL,connfd,NULL,32768,
                    SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);

        close(connfd);
    }
    close(sock);
    return 0;
}
