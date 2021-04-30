#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        std::cout << "usage: "<< argv[0] <<" ip port_number\n";
        return 1;
    }
    
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in server_address;
    bzero(&server_address,sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&server_address.sin_addr);
    server_address.sin_port =  htons(port);

    int sock_fd = socket(AF_INET,SOCK_STREAM,0);
    assert(sock_fd >= 0);

    int ret = connect(sock_fd,(sockaddr*)&server_address,
                        sizeof(server_address));
    
    if(ret < 0)
    {
        std::cout<<"connection failed\n";
    }
    else{
        const char* oob_data = "abc";
        const char* normal_data = "123";
        send(sock_fd,normal_data,strlen(normal_data),0);
        send(sock_fd,oob_data,strlen(oob_data),MSG_OOB);
        send(sock_fd,normal_data,strlen(normal_data),0);
    }
    close(sock_fd);
    return 0;
}
