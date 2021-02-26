
#include <muduo/base/ThreadPool.h>
#include <muduo/base/Exception.h>

#include <assert.h>
#include <stdio.h>

using namespace muduo;

ThreadPool::ThreadPool(const string& name) :
    _mutex(),
    _not_empty(_mutex),
    _not_full(_mutex),
    _name(name),
    _queue_capacity(0),
    _running(false) {}

ThreadPool::~ThreadPool() {
    if (_running) {
        stop();
    }
}

void ThreadPool::start(int threads_num) {
    assert(_threads.empty());
    _running = true;
    _threads.reserve(threads_num);
    for (int i = 0; i < threads_num; ++i) {
        char id[32];
        snprintf(id, sizeof(id), "%d", i + 1);
        _threads.emplace_back(new muduo::Thread(std::bind(&ThreadPool::runInThread, this), _name + id));
        _threads[i]->start();
    }
    if (threads_num == 0 && _thread_init_callback) {
        _thread_init_callback();
    }
}

void ThreadPool::stop() {
    {
        MutexLockGuard lock(_mutex);
        _running = false;
        _not_empty.notifyAll();
    }

    for (auto& thread : _threads) {
        thread->join();
    }
}

size_t ThreadPool::queueSize() const {
    MutexLockGuard lock(_mutex);
    return _queue.size();
}

void ThreadPool::run(Task task) {
    if (_threads.empty()) {
        task();
    }
    else {
        MutexLockGuard lock(_mutex);
        while (isFull()) {
            _not_full.wait();
        }
        assert(!isFull());

        _queue.push_back(std::move(task));
        _not_empty.notify();
    }
}

ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(_mutex);
    while (_queue.empty() && _running) {
        _not_empty.wait();
    }
    Task task;
    if (!_queue.empty()) {
        task = _queue.front();
        _queue.pop_front();
        if (_queue_capacity > 0) {
            _not_full.notify();
        }
    }

    return task;
}

bool ThreadPool::isFull() const {
    _mutex.assertLocked();
    return _queue_capacity > 0 && _queue.size() >= _queue_capacity;
}   

void ThreadPool::runInThread() {
    try {
        if (_thread_init_callback) {
            _thread_init_callback();
        }

        while (_running) {
            Task task(take());
            if (task) {
                task();
            }
        }
    }
    catch (const Exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", _name.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
    catch (const std::exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", _name.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...) {
        fprintf(stderr, "unknow exception caught in ThreadPool %s\n", _name.c_str());
        throw;
    }
}