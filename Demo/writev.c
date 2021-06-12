#include <stdio.h>
#include <string.h>
#include <sys/uio.h>

int main(int argc, char *argv[])
{
    
    char* str1 = "hello ";
    char* str2 = "world\n";

    struct iovec iov[2];
    
    iov[0].iov_base = str1;
    iov[0].iov_len = strlen(str1);
    iov[1].iov_base = str2;
    iov[1].iov_len = strlen(str2);

    writev(1,iov,2);
    return 0;
}
