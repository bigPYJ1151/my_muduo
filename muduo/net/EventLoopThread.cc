
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const string& name) :
    _loop(NULL),
    _exiting(false),
    _thread(std::bind(&EventLoopThread::threadFunc, this), name),
    _mutex(),
    _cond(_mutex),
    _callback(cb) {}

EventLoopThread::~EventLoopThread() {
    _exiting = true;
    if (_loop != NULL) {
        _loop->quit();
        _thread.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!_thread.started());
    _thread.start();

    EventLoop* loop = NULL;
    {
        MutexLockGuard lock(_mutex);
        while (_loop == NULL) {
            _cond.wait();
        }
        loop = _loop;
    }

    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (_callback) {
        _callback(&loop);
    }

    {
        MutexLockGuard lock(_mutex);
        _loop = &loop;
        _cond.notify();
    }

    loop.loop();
    MutexLockGuard lock(_mutex);
    _loop = NULL;
}