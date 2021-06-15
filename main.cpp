#include "./Server/server.h"

int main(int argc, char const *argv[])
{
    Server* server = new Server(8888,3,30000,true,3306,"root",
                     "", "TestDB", 12,8,true, 0,1024);

    
    server->Start();

    delete server;
    return 0;
}
