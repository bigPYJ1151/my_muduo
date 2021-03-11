#ifndef WEB_SERVER_TIMEWHEEL_H
#define WEB_SERVER_TIMEWHEEL_H

#include <muduo/base/Logging.h>
#include <muduo/net/TimerId.h>
#include <muduo/net/TcpConnection.h>

#include <memory>
#include <vector>

using namespace muduo::net;
typedef std::weak_ptr<TcpConnection> weakTcpConnectionPtr;

class TimeWheelEntry {
public:
    TimeWheelEntry(const TcpConnectionPtr& ptr) : wptr_(ptr) {}
    ~TimeWheelEntry() {
        TcpConnectionPtr conn = wptr_.lock();
        if (conn) {
            conn->shutdown();
        }
    }

private:
    const weakTcpConnectionPtr wptr_;
};

typedef std::shared_ptr<TimeWheelEntry> TimeWheelEntryPtr;
typedef std::weak_ptr<TimeWheelEntry> weakTimeWheelEntryPtr;


class TimerWheel {
public:
    TimerWheel(int second, EventLoop* loop);

    void onTimer();

    void start();

    void stop();

    bool isRunning() {
        return run_;
    }

    void insertEntry(TimeWheelEntryPtr entry);    

private:
    void insertEntryInLoop(TimeWheelEntryPtr entry) {
        int next = next_second - 1;
        next = next < 0 ? next + 1 : next;
        LOG_INFO << "TimerWheel insert at " << next;
        wheel_[next].push_back(entry);
    }

    const int max_second;
    int next_second;
    std::vector<std::vector<TimeWheelEntryPtr>> wheel_;
    EventLoop* loop_;
    TimerId id_;
    bool run_;
};

#endif
