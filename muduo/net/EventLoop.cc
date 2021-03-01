
#include <muduo/net/EventLoop.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/Channel.h>
#include <muduo/net/Poller.h>
#include <muduo/net/SocketsOps.h>

#include <algorithm>

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

namespace {

__thread EventLoop* t_loop_in_this_thread = 0;

const int k_poll_time_ms = 10000;

int createEventfd() {
    int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd < 0) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return event_fd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe init_obj;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loop_in_this_thread;
}

EventLoop::EventLoop() : 
    _looping(false),
    _quit(false),
    _event_handling(false),
    _calling_pending_functors(false),
    _iteration(0),
    _thread_id(CurrentThread::tid()),
    _poller(Poller::newDefaultPoller(this)),
    _wake_up_fd(createEventfd()),
    _wake_up_channel(new Channel(this, _wake_up_fd)),
    _current_active_channel(NULL)
{
    LOG_DEBUG << "EventLoop created " << this << " in thread " << _thread_id;
    if (t_loop_in_this_thread) {
        LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread << " exists in this thread " << _thread_id;
    }
    else {
        t_loop_in_this_thread = this;
    }
    _wake_up_channel->setReadCallback(
        std::bind(&EventLoop::handleRead, this)
    );
    _wake_up_channel->enableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << this << " of thread " << _thread_id
    << " destructs in thread " << CurrentThread::tid();
    _wake_up_channel->disableAll();
    _wake_up_channel->remove();
    ::close(_wake_up_fd);
    t_loop_in_this_thread = NULL;
}

void EventLoop::loop() {
    assert(!_looping);
    assertInLoopThread();
    _looping = true;
    _quit = false;
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!_quit) {
        _active_channels.clear();
        _poll_return_time = _poller->poll(k_poll_time_ms, &_active_channels);
        ++_iteration;
        if (Logger::logLevel() <= Logger::TRACE) {
            printActiveChannels();
        }

        _event_handling = true;
        for (Channel* channel : _active_channels) {
            _current_active_channel = channel;
            _current_active_channel->handleEvent(_poll_return_time);
        }
        _current_active_channel = NULL;
        _event_handling = false;
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    _looping = false;
}

void EventLoop::quit() {
    _quit = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInloop(std::move(cb));
    }
}

void EventLoop::queueInloop(Functor cb) {
    {
        MutexLockGuard lock(_mutex);
        _pending_functors.push_back(std::move(cb));
    }

    if (!isInLoopThread() || _calling_pending_functors) {
        wakeup();
    }
}

size_t EventLoop::queueSize() const {
    MutexLockGuard lock(_mutex);
    return _pending_functors.size();
}

// Channel functions
void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (_event_handling) {
        assert(_current_active_channel == channel ||
            std::find(_active_channels.begin(), _active_channels.end(), channel) == _active_channels.end());
    }
    _poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return _poller->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this <<
    " was created in _thread_id = " << _thread_id <<
    ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = sockets::write(_wake_up_fd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = sockets::read(_wake_up_fd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    _calling_pending_functors = true;
    {
        MutexLockGuard lock(_mutex);
        functors.swap(_pending_functors);
    }
    for (const Functor& functor : functors) {
        functor();
    }
    _calling_pending_functors = false;
}

void EventLoop::printActiveChannels() const {
    for (const Channel* channel: _active_channels) {
        LOG_TRACE << "{" << channel->reventsToString() << "}";
    }
}