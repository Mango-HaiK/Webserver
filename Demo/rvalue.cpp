#include <iostream>

void show_one(int &n)
{
    std::cout << "show_one(int&) :" << n << std::endl;
}

void show_one(int &&n)
{
    std::cout << "show_one(int&&) :" << n << std::endl;
}

void forward(int &&n)
{
    //show_one(n);                    //调用show_one(int&)
    show_one(std::forward<int>(n));   //调用show_one(int&&)
}

int main(int argc, char const *argv[])
{
    int n = 1;
    show_one(n);            //show_one(int&): 1
    show_one(std::move(n)); //show_one(int&&): 1
    show_one(1);            //show_one(int&&): 1
    
    forward(1);             

    return 0;
}
