
#ifndef WEB_SERVER_SERVER_H
#define WEB_SERVER_SERVER_H

#include <muduo/net/TcpServer.h>
#include <web_server/Request.h>
#include <web_server/Response.h>
#include <web_server/TimeWheel.h>
#include <web_server/ContextParse.h>

using namespace muduo::net;
typedef std::pair<ContextParse, weakTimeWheelEntryPtr> ContextType;

class Server {
public:
    using Timestamp = muduo::Timestamp;
    typedef std::function<void (const Request&, Response*)> HttpCallback;

    Server(EventLoop* loop,
            const InetAddress& listen_addr,
            const std::string& name,
            TcpServer::Option option = TcpServer::kNoReusePort);
    
    EventLoop* getLoop() const {
        return server_.getLoop();
    }

    void setThreadNum(int num_thread) {
        server_.setThreadNum(num_thread);
    }

    void start();

    void setHttpCallback(const HttpCallback& cb) {
        http_callback_ = cb;
    }

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receive_time);
    void onRequest(const TcpConnectionPtr&, const Request&);

    TcpServer server_;
    HttpCallback http_callback_;
    TimerWheel timer_;
};


#endif