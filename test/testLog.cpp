#include <iostream>
#include "../Log/log.h"

int main(int argc, char const *argv[])
{
    for (int i = 0; i < 10000; i++)
    {
        LOG_DEBUG("[%d]:hello this is test...",i);
    }

    return 0;
}
