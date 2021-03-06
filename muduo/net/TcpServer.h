
#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include <muduo/base/Atomic.h>
#include <muduo/base/Types.h>
#include <muduo/net/TcpConnection.h>

#include <map> 

namespace muduo {
namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable {
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;
    enum Option {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop,
              const InetAddress& listen_addr,
              const string& name,
              Option options = kNoReusePort);
    ~TcpServer();

    const string& ipPort() const {
        return _ip_port;
    }

    const string& name() const {
        return _name;
    }

    EventLoop* getLoop() const {
        return _loop;
    }

    void setThreadNum(int num_threads);
    void setThreadInitCallback(const ThreadInitCallback& cb) {
        _thread_init_callback = cb;
    }

    std::shared_ptr<EventLoopThreadPool> threadPool() {
        return _thread_pool;
    }
    
    void start();
    void setConnectionCallback(const ConnectionCallback& cb) {
        _connection_callback = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        _message_callback = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        _write_complete_callback = cb;
    }

private:
    void newConnection(int sockfd, const InetAddress& peer_addr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    typedef std::map<string, TcpConnectionPtr> ConnectionMap;

    EventLoop* _loop;
    const string _ip_port;
    const string _name;
    std::unique_ptr<Acceptor> _acceptor;
    std::shared_ptr<EventLoopThreadPool> _thread_pool;
    ConnectionCallback _connection_callback;
    MessageCallback _message_callback;
    WriteCompleteCallback _write_complete_callback;
    ThreadInitCallback _thread_init_callback;
    AtomicInt32 _started;
    int _next_conn_id;
    ConnectionMap _connections;
};

}
}

#endif
