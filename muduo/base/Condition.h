#ifndef MUDUO_BASE_CONDITION_H
#define MUDUO_BASE_CONDITION_H

#include <muduo/base/Mutex.h>

#include <pthread.h>

namespace muduo {
class Condition : noncopyable {
public:
    explicit Condition(MutexLock& mutex) : _mutex(mutex) {
        MCHECK(pthread_cond_init(&_pcond, NULL));
    }

    ~Condition() {
        MCHECK(pthread_cond_destroy(&_pcond));
    }

    void wait() {
        MutexLock::UnassignGuard ug(_mutex);
        MCHECK(pthread_cond_wait(&_pcond, _mutex.getPthreadMutex()));
    }
    
    // return true if timeout, false otherwise
    bool waitForSeconds(double seconds);

    void notify() {
        MCHECK(pthread_cond_signal(&_pcond));
    }
    
    void notifyAll() {
        MCHECK(pthread_cond_broadcast(&_pcond));
    }
    
private:
    MutexLock& _mutex;
    pthread_cond_t _pcond;
};
}

#endif