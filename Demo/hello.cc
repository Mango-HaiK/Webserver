#include <iostream>
#include <sys/types.h>
#include <csignal>

using namespace std;

int main(int argc, char const *argv[])
{
    clock_t start = clock();
    std::cout<<"hello world"<<std::endl;
    cout <<start - clock()<<endl;
    return 0;
}
