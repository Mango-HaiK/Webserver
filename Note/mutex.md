## Mutex
Mutex主要有void lock()、void unlock()、bool try_lock()
* lock() 加锁，不允许其他线程访问在使用unlock前的代码段，如锁已被占有则阻塞等待直至获得该锁
* try_lock() 申请锁时如该锁不已被占有，则立即返回false

C++11提供了更安全的Mutex，