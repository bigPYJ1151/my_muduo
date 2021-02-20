#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include <muduo/base/Atomic.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Types.h>

#include <functional>
#include <memory>
#include <pthread.h>

namespace muduo {
class Thread : noncopyable {
public:
    typedef std::function<void ()> ThreadFunc;

    explicit Thread(ThreadFunc, const string& name = string());
    ~Thread();

    void start();
    int join();

    bool started() const {
        return _started;
    }

    pid_t tid() const {
        return _tid;
    }

    const string& name() const {
        return _name;
    }

    static int numCreated() {
        return _num_created.get();
    }

private:
    void setDefaultName();
    
    bool _started;
    bool _joined;
    pthread_t _pthread_id;
    pid_t _tid;
    ThreadFunc _func;
    string _name;
    CountDownLatch _latch;

    static AtomicInt32 _num_created;
};
}

#endif