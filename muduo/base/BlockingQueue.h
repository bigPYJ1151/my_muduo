#ifndef MUDUO_BASE_BLOCKINGQUEUE_H
#define MUDUO_BASE_BLOCKINGQUEUE_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>

#include <deque>
#include <assert.h>

namespace muduo {
template <typename T>
class BlockingQueue : noncopyable {
public:
    BlockingQueue() : _mutex(), _not_empty(_mutex), _queue() {}

    void put(const T& x) {
        MutexLockGuard lock(_mutex);
        _queue.push_back(x);
        _not_empty.notify();
    }

    void put(T&& x) {
        MutexLockGuard lock(_mutex);
        _queue.push_back(std::move(x));
        _not_empty.notify();
    }

    T take() {
        MutexLockGuard lock(_mutex);
        while (_queue.empty()) {
            _not_empty.wait();
        }

        assert(!_queue.empty());
        T ans(std::move(_queue.front()));
        _queue.pop_front();

        return ans;
    }

    size_t size() const {
        MutexLockGuard lock(_mutex);

        return _queue.size();
    }

private:
    mutable MutexLock _mutex;
    Condition _not_empty GUARDED_BY(_mutex);
    std::deque<T> _queue GUARDED_BY(_mutex);
};

}

#endif