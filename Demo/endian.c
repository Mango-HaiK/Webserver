/**
 * 判断机器字节序
 * 大段序 -> 高位字节存放在内存低地址处
 * 小段序 -> 低位字节存放在内存底地址处
 * */
#include <stdio.h>
#include <bits/socket.h>

int main(int argc, char const *argv[])
{
    union{
        short value;
        char union_bytes[sizeof(short)];
    }test;
    test.value = 0x0102;
    if(test.union_bytes[0] == 1)
    {
        printf("This is big endian.\n");
    }else if (test.union_bytes[0] == 2)
    {
        printf("This is little endian.\n");
    }else
    {
        printf("unknow.\n");
    }
    return 0;
}
