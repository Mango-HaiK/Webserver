#include <iostream>
#include <unistd.h>

using namespace std;
int main(int argc, char const *argv[])
{
    int n;
    int pfd[2];
    pid_t pid;
    char buf[1024];

    if (pipe(pfd) < 0)
    {
        cerr<<"fork erron\n";
    }
    
    if ((pid = fork()) < 0)
    {
        cerr<<"fork erron\n";
    }else if(pid == 0)  //parent
    {
        close(pfd[0]);
        write(pfd[1],"hello word\n",12);
    }else if (pid > 0)  //child
    {
        close(pfd[1]);
        n = read(pfd[0],buf,1024);
        write(STDOUT_FILENO,buf,n);
    }
    exit(0);
}
