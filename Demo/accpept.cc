#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    if (argc <= 2)
    {
        std::cout<<"usage: "<< argv[0] <<" ip port_number\n";
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sock = socket(AF_INET,SOCK_STREAM,0);
    assert(sock >= 0);

    int ret = bind(sock,(sockaddr *)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(sock,5);
    assert(ret != -1);

    sleep(30);

    sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int connfd = accept(sock,(sockaddr*)&client,&client_len);
    
    if(connfd < 0)
        std::cout<<"erron is: "<< errno;
    else
    {
        char remote[INET_ADDRSTRLEN];
        const char* client_addr = inet_ntop(AF_INET,&client.sin_addr,remote,INET_ADDRSTRLEN);
        std::cout<<"connected with ip: "<<client_addr
                    <<" and "<< ntohs(client.sin_port) << std::endl;
        close(connfd);
    }
    close(sock);
    return 0;
}
