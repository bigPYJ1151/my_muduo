#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H

#include <muduo/base/noncopyable.h>
#include <muduo/base/Types.h>

#include <functional>
#include <memory>
#include <vector>

namespace muduo {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop* base_loop, const string& name);
    ~EventLoopThreadPool();
    void setThreadNum(int num) {
        _num_threads = num;
    }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    // round-robin
    EventLoop* getNextLoop();
    EventLoop* getLoopForHash(size_t hash_code);

    std::vector<EventLoop*> getAllLoops();

    bool started() const {
        return _started;
    }

    const string& name() const {
        return _name;
    }

private:
    EventLoop* _base_loop;
    string _name;
    bool _started;
    int _num_threads;
    int _next;
    std::vector<std::unique_ptr<EventLoopThread>> _threads;
    std::vector<EventLoop*> _loops;
};

}
}

#endif