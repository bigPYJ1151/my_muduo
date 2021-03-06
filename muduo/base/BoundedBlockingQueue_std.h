
#include <mutex>
#include <condition_variable>
#include <deque>
#include <assert.h>

namespace muduo {

template <typename T>
class BoundedBlockingQueue_std {
public:
    BoundedBlockingQueue_std(const BoundedBlockingQueue_std&) = delete;
    BoundedBlockingQueue_std& operator=(const BoundedBlockingQueue_std&) = delete;

    explicit BoundedBlockingQueue_std(size_t capacity) :
        _capacity(capacity)
         {}

    void put(const T& x) {
        std::unique_lock lk(_mutex);
        while (_queue.size() == _capacity) {
            _not_full.wait(lk);
        }
        assert(_queue.size() < _capacity);
        _queue.push_back(x);
        _not_empty.notify_one();
    }

    void put(T&& x) {
        std::unique_lock lk(_mutex);
        while (_queue.size() == _capacity) {
            _not_full.wait(lk);
        }
        assert(_queue.size() < _capacity);
        _queue.push_back(std::move(x));
        _not_empty.notify_one();
    }

    T take() {
        std::unique_lock lk(_mutex);
        while (_queue.empty()) {
            _not_empty.wait(lk);
        }
        assert(!_queue.empty());
        T ans(std::move(_queue.front()));
        _queue.pop_front();
        _not_full.notify_one();

        return ans;
    }

    bool full() const {
        std::unique_lock lk(_mutex);
        return _queue.size() == _capacity;
    }

    bool empty() const {
        std::unique_lock lk(_mutex);
        return _queue.empty();
    }

    size_t size() const {
        std::unique_lock lk(_mutex);
        return _queue.size();
    }

    size_t capacity() const {
        std::unique_lock lk(_mutex);
        return _capacity;
    }
    
private:
    const size_t _capacity;
    mutable std::mutex _mutex;
    std::condition_variable _not_full;
    std::condition_variable _not_empty;
    std::deque<T> _queue;
};
}

