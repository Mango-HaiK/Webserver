#include "../Log/log.h"
#include "threadPool.h"

#include "sqlConnPool.h"

#include <sstream>

void ThreadLogTask(int i, int cnt) {
    for(int j = 0; j < 10000; j++ ){
        auto id = std::this_thread::get_id();
        std::stringstream s;
        s << id;
        std::string sid = s.str();
        LOG_BASE(i,"PID:[%s]======= %05d ========= ", sid.c_str(), cnt++);
    }
}

void TestThreadPool() {
    Log::Instance()->Init(0, "./testThreadpool", ".log", 5000);
    ThreadPool threadpool(6);
    for(int i = 0; i < 18; i++) {
        threadpool.AddTask(std::bind(ThreadLogTask, i % 4, i));
    }
    getchar();
}

int main() {
    TestThreadPool();
    //SqlConnPool::getInstace().Init("127.0.0.1",3306,"root","","TestDB",8);
    

}