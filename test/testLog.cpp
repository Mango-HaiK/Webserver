#include <iostream>
#include "../Log/log.h"

int main(int argc, char const *argv[])
{
    Log::Instance()->Init(0,"./log",".log",12);
    for (int i = 0; i < 60000; i++)
    {
        LOG_DEBUG("[%05d]:hello this is test...",i);
    }

    return 0;
}
