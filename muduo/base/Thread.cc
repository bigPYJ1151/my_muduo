
#include <muduo/base/Thread.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Exception.h>

#include <type_traits>

#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace muduo {
namespace detial {
pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork() {
    muduo::CurrentThread::t_cached_tid = 0;
    muduo::CurrentThread::t_thread_name = "main";
    CurrentThread::tid();
}

class ThreadNameInitializer {
public:
    ThreadNameInitializer() {
        muduo::CurrentThread::t_thread_name = "main";
        CurrentThread::tid();
        pthread_atfork(NULL, NULL, &afterFork);
    }
};

ThreadNameInitializer init;

struct ThreadData {
    typedef muduo::Thread::ThreadFunc ThreadFunc;
    ThreadFunc _func;
    string _name;
    pid_t* _tid;
    CountDownLatch* _latch;

    ThreadData(ThreadFunc func, 
                const string& name,
                pid_t* tid,
                CountDownLatch* latch) :
                _func(std::move(func)),
                _name(name),
                _tid(tid),
                _latch(latch) {}

    void runInThread() {
        *_tid = muduo::CurrentThread::tid();
        _tid = NULL;
        _latch->countDown();
        _latch = NULL;

        muduo::CurrentThread::t_thread_name = _name.empty() ? "muduoThread" : _name.c_str();
        ::prctl(PR_SET_NAME, muduo::CurrentThread::t_thread_name);

        try {
            _func();
            muduo::CurrentThread::t_thread_name = "finished";
        }
        catch (const Exception& ex) {
            muduo::CurrentThread::t_thread_name = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", _name.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
            abort();
        }
        catch (const std::exception& ex) {
            muduo::CurrentThread::t_thread_name = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", _name.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...) {
            muduo::CurrentThread::t_thread_name = "crashed";
            fprintf(stderr, "unknow exception caught in Thread %s\n", _name.c_str());
            throw;
        }
    }
};

void* startThread(void* obj) {
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}
}

void CurrentThread::cacheTid() {
    if (t_cached_tid == 0) {
        t_cached_tid = detial::gettid();
        t_tid_string_len = snprintf(t_tid_string, sizeof(t_tid_string), "%5d ", t_cached_tid);
    }
}

bool CurrentThread::isMainThread() {
    return tid() == ::getpid();
}

// void CurrentThread::sleepUsec(int64_t usec) {
//     struct timespec ts = {0, 0};
//     ts.tv_sec = static_cast<time_t>(usec / )
// }

AtomicInt32 Thread::_num_created;

Thread::Thread(ThreadFunc func, const string& name) :
                _started(false),
                _joined(false),
                _pthread_id(0),
                _tid(0),
                _func(std::move(func)),
                _name(name),
                _latch(1)                       {
    setDefaultName();
}

Thread::~Thread() {
    if (_started && !_joined) {
        pthread_detach(_pthread_id);
    }
}

void Thread::setDefaultName() {
    int num = _num_created.incrementAndGet();
    if (_name.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        _name = buf;
    }
}

void Thread::start() {
    assert(!_started);
    _started = true;
    detial::ThreadData* data = new detial::ThreadData(_func, _name, &_tid, &_latch);
    if (pthread_create(&_pthread_id, NULL, &detial::startThread, data)) {
        _started = false;
        delete data;
        // TODO: logging
    }
    else {
        _latch.wait();
        assert(_tid > 0);
    }
}

int Thread::join() {
    assert(_started);
    assert(!_joined);
    _joined = true;

    return pthread_join(_pthread_id, NULL);
}
}