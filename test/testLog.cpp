#include <iostream>
#include "../Log/log.h"

int main(int argc, char const *argv[])
{
    Log::Instance()->Init(0,"./log",".log",12);
    for (int i = 0; i < 60; i++)
    {
        sleep(2);
        LOG_DEBUG("[%05d]:hello this is test...",i);
        LOG_ERROR("[%05d]:hello this is test...",i);
        LOG_INFO("[%05d]:hello this is test...",i);
        LOG_WARN("[%05d]:hello this is test...",i);
    }

    return 0;
}
