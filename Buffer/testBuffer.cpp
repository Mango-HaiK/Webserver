#include <iostream>
#include <string>
#include <vector>

#include "buffer.h"

using namespace std;

int main(int argc, char const *argv[])
{
    vector<char> str0 = {'h','e','l','l','o'};
    cout<<&*str0.begin()<<endl;


    Buffer buff;
    const char* str = "hello word!";
    buff.Append(str,strlen(str));

    cout << "buff.Peek(): " << buff.Peek()<< endl;
    
    cout<< "buff.WritableBytes: " <<buff.WritableBytes() <<endl;
    cout<< "buff.ReadableBytes: " <<buff.ReadableBytes() <<endl;

    cout<< "buff.RetrieveToStr: " <<buff.RetrieveToStr()<<endl;
    cout<<buff.Peek()<<endl;
    return 0;
}
