
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>

#include <stdio.h>

using namespace muduo;
using namespace net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, const string& name) :
    _base_loop(base_loop),
    _name(name),
    _started(false),
    _num_threads(0),
    _next(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    assert(!_started);
    _base_loop->assertInLoopThread();
    _started = true;
    for (int i = 0; i < _num_threads; ++i) {
        char buf[_name.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", _name.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        _threads.push_back(std::unique_ptr<EventLoopThread>(t));
        _loops.push_back(t->startLoop());
    }
    if (_num_threads == 0 && cb) {
        cb(_base_loop);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    _base_loop->assertInLoopThread();
    assert(_started);
    EventLoop* loop = _base_loop;

    if (!_loops.empty()) {
        // round-robin
        loop = _loops[_next];
        ++_next;
        if (implicit_cast<size_t>(_next) >= _loops.size()) {
            _next = 0;
        }
    }

    return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hash_code) {
    _base_loop->assertInLoopThread();
    EventLoop* loop = _base_loop;

    if (!_loops.empty()) {
        loop = _loops[hash_code % _loops.size()];
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    _base_loop->assertInLoopThread();
    assert(_started);
    if (_loops.empty()) {
        return std::vector<EventLoop*>(1, _base_loop);
    }
    else {
        return _loops;
    }
}