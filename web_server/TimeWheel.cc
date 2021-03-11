
#include <muduo/net/EventLoop.h>
#include <web_server/TimeWheel.h>

#include <assert.h>
#include <functional>

TimerWheel::TimerWheel(int second, EventLoop* loop) : 
    max_second(second), next_second(0), wheel_(second), loop_(loop), run_(false) {
}

void TimerWheel::start() {
    assert(!run_);
    LOG_TRACE << "TimerWheel started.";
    run_ = true;
    id_ = loop_->runEvery(1.0, std::bind(&TimerWheel::onTimer, this));
}

void TimerWheel::stop() {
    assert(run_);
    LOG_TRACE << "TimerWheel cancaled.";
    run_ = false;
    loop_->cancel(id_);
}

void TimerWheel::onTimer() {
    LOG_TRACE << "TimerWheel tick at " << next_second << " Clear Num: " << wheel_[next_second].size() ;
    wheel_[next_second].clear();

    next_second = next_second + 1;
    next_second %= max_second;
}

void TimerWheel::insertEntry(TimeWheelEntryPtr entry) {
    if (run_) {
        loop_->runInLoop(std::bind(&TimerWheel::insertEntryInLoop, this, entry));
    }
}