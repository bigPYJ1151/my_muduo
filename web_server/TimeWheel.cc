
#include <muduo/net/EventLoop.h>
#include <web_server/TimeWheel.h>

#include <functional>

TimerWheel::TimerWheel(int second, EventLoop* loop) : 
    max_second(second), next_second(0), wheel_(second), loop_(loop) {
    
    id_ = loop_->runEvery(1.0, std::bind(&TimerWheel::onTimer, this));
}

void TimerWheel::onTimer() {
    LOG_INFO << "Timer! " << "Clear: " << next_second;
    wheel_[next_second].clear();

    next_second = next_second + 1;
    next_second %= max_second;
}

void TimerWheel::insertEntry(TimeWheelEntryPtr entry) {
    loop_->runInLoop(std::bind(&TimerWheel::insertEntryInLoop, this, entry));
}