#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char const *argv[])
{
    const char* str1 = "hel";
    const char* str2 = "world";
    std::string line(str1,str2);
    std::cout << line << std::endl;

    vector<char> buffer(1024);
    buffer = "hello";
    *buffer.begin();

    return 0;
}
