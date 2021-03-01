#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <any>
#include <atomic>
#include <functional>
#include <vector>

#include <muduo/base/Mutex.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>

namespace muduo {
namespace net {

class Channel;
class Poller;

class EventLoop : noncopyable {
public:
    typedef std::function<void ()> Functor;

    EventLoop();
    ~EventLoop();

    // Loop forever, must be called in creator thread
    void loop();

    //Quits loop, recommend to be called through shared_ptr
    void quit();

    // Time when poll returns
    Timestamp pollReturnTime() const {
        return _poll_return_time;
    }

    int64_t iteration() const {
        return _iteration;
    }

    // Runs callback immediately in loop thread
    // It wakes up the loop and run callback function
    // Safe to call from other threads
    void runInLoop(Functor cb);

    // Queues callback int the loop thread
    // Runs after finish loop
    // Safe to call from other threads
    void queueInloop(Functor cb);

    size_t queueSize() const;

    // Internal usage
    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const {
        return _thread_id == CurrentThread::tid();
    }

    bool eventHandling() const {
        return _event_handling;
    }

    void setContext(const std::any& context) {
        _context = context;
    }

    const std::any& getContext() const {
        return _context;
    }

    std::any* getMutableContext() {
        return &_context;
    }

    static EventLoop* getEventLoopOfCurrentThread();
    
private:
    void abortNotInLoopThread();
    // Waked up
    void handleRead();
    void doPendingFunctors();

    void printActiveChannels() const;

    typedef std::vector<Channel*> ChannelList;

    bool _looping;
    std::atomic<bool> _quit;
    bool _event_handling;
    bool _calling_pending_functors;
    int64_t _iteration;
    const pid_t _thread_id;
    Timestamp _poll_return_time;
    std::unique_ptr<Poller> _poller;
    int _wake_up_fd;

    std::unique_ptr<Channel> _wake_up_channel;
    std::any _context;

    ChannelList _active_channels;
    Channel* _current_active_channel;

    mutable MutexLock _mutex;
    std::vector<Functor> _pending_functors GUARDED_BY(_mutex);
};

}
}

#endif