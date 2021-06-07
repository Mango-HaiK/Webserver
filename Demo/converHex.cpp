#include <iostream>

int ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

int main(int argc, char const *argv[])
{
    char ch = 'R';
    std::cout<<ConverHex(ch)<<std::endl;
    return 0;
}
