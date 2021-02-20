#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>

namespace muduo {
class CountDownLatch : noncopyable {
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();
    int getCount() const;
    
private:
    mutable MutexLock _mutex;
    Condition _condition GUARDED_BY(_mutex);
    int _count GUARDED_BY(_mutex);
};
}

#endif