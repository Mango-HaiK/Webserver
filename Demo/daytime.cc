#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    assert(argc == 2);
    char *host = argv[1];
    
    //获取目标主机信息
    hostent *host_info = gethostbyname(host);
    assert(host_info);
    
    servent* serv_info = getservbyname("daytime","tcp");
    assert(serv_info);
    printf("daytime port is %d \n",ntohs(serv_info->s_port));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = serv_info->s_port;
    address.sin_addr = *(in_addr*) *host_info->h_addr_list;

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    int ret = connect(sockfd,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    char buffer[128];
    ret = read(sockfd,&buffer,sizeof(buffer));
    assert(ret > 0);
    buffer[ret] = '\0';
    printf("the day time is : %s .\n",buffer);
    close(sockfd);
    return 0;
}
