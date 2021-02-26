#ifndef MUDUO_BASE_THREADPOOL_H
#define MUDUO_BASE_THREADPOOL_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Types.h>

#include <deque>
#include <vector>

namespace muduo {
class ThreadPool : noncopyable {
public:
    typedef std::function<void ()> Task;

    explicit ThreadPool(const string& name = string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(size_t capacity) {
        _queue_capacity = capacity;
    }

    void setThreadInitCallback(const Task& callback) {
        _thread_init_callback = callback;
    }

    void start(int threads_num);
    void stop();

    const string& name() const {
        return _name;
    }

    size_t queueSize() const;

    void run(Task f);
    
private:
    bool isFull() const REQUIRES(_mutex);
    void runInThread();
    Task take();

    mutable MutexLock _mutex;
    Condition _not_empty GUARDED_BY(_mutex);
    Condition _not_full GUARDED_BY(_mutex);
    string _name;
    Task _thread_init_callback;
    std::vector<std::unique_ptr<muduo::Thread>> _threads;
    std::deque<Task> _queue GUARDED_BY(_mutex);
    size_t _queue_capacity;
    bool _running;
};
}

#endif
