#include <iostream>
#include <string>

class string
{
public:
    string(char* str):str_(nullptr)
    {
        str_ = new char[strlen(str) + 1];
        str[strlen(str) + 1] = '\0';
        //strcpy(str_,str);
        //while ((*str_++ = *str++) != '\0');
        str_ = str;
        while ((*str_++ = *str++) != '\0'); 
    }

    char* get_str(){return str_;}

    char* str_;
};


int main(int argc, char const *argv[])
{
    char s[6] = "hello";
    char* s1 = s;
    std::cout<<s<<std::endl;
    std::cout<<strlen(s)<<" "<<sizeof(s) << std::endl;
    std::cout<<strlen(s1)<<" "<<sizeof(s1) << std::endl;

    s1++;
    char ch = *(s1++);
    std::cout<<"ch = " << ch << std::endl;

    string str(s);
    std::cout<< str.get_str()<<std::endl;

    return 0;
}
