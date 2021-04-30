#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include <iostream>

using namespace std;

static bool stop = false;

//触发时结束循环
static void handle_term(int sig)
{
    stop = true;
}
int main(int argc, char const *argv[])
{
    signal(SIGTERM,handle_term);

    if(argc <= 3)
    {
        cout<<"usage: "<<argv[0]<<" ip_address port_number backlog\n";
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);

    int sock = socket(PF_INET,SOCK_STREAM,0);
    assert(sock >= 0);

    //creat a ipv4 socket
    sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    int rec = bind(sock,(sockaddr*)&address,sizeof(address));
    if(rec == -1)
        cout<<"bind erro\n"<<errno;

    rec = listen(sock,backlog);
    assert(rec != -1);

    while (!stop)
    {
        sleep(1);
    }

    close(sock);

    return 0;
}
