#ifndef MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
#define MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>

#include <deque>
#include <assert.h>

namespace muduo {
template <typename T>
class BoundedBlockingQueue : noncopyable {
public:
    explicit BoundedBlockingQueue(size_t capacity)
        : _capacity(capacity), 
        _mutex(),
        _not_empty(_mutex),
        _not_full(_mutex) {}

    void put(const T& x) {
        MutexLockGuard lock(_mutex);
        while (_queue.size() == _capacity) {
            _not_full.wait();
        }
        assert(_queue.size() < _capacity);
        _queue.push_back(x);
        _not_empty.notify();
    }

    void put(T&& x) {
        MutexLockGuard lock(_mutex);
        while (_queue.size() == _capacity) {
            _not_full.wait();
        }
        assert(_queue.size() < _capacity);
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
        _not_full.notify();

        return ans;
    }

    bool full() const {
        MutexLockGuard lock(_mutex);
        return _queue.size() == _capacity;
    }

    bool empty() const {
        MutexLockGuard lock(_mutex);
        return _queue.empty(); 
    }

    size_t size() const {
        MutexLockGuard lock(_mutex);
        return _queue.size();
    }

    size_t capacity() const {
        MutexLockGuard lock(_mutex);
        return _capacity;
    }
private:
    const size_t _capacity;
    mutable MutexLock _mutex;
    Condition _not_empty GUARDED_BY(_mutex);
    Condition _not_full GUARDED_BY(_mutex);
    std::deque<T> _queue GUARDED_BY(_mutex);
};
}

#endif