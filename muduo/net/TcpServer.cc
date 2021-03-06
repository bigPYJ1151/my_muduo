
#include <muduo/net/TcpServer.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Acceptor.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

TcpServer::TcpServer(EventLoop* loop,
                    const InetAddress& listen_addr,
                    const string& name_arg,
                    Option option) :
            _loop(CHECK_NOTNULL(loop)),
            _ip_port(listen_addr.toIpPort()),
            _name(name_arg),
            _acceptor(new Acceptor(loop, listen_addr, option == kReusePort)),
            _thread_pool(new EventLoopThreadPool(loop, _name)),
            _connection_callback(defaultConnectionCallback),
            _message_callback(defaultMessageCallback),
            _next_conn_id(1) {
    
    _acceptor->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2)
    );
}

TcpServer::~TcpServer() {
    _loop->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << _name << "] destructing";
    for (auto& item : _connections) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::setThreadNum(int num_threads) {
    assert(0 <= num_threads);
    _thread_pool->setThreadNum(num_threads);
}

void TcpServer::start() {
    if (_started.getAndSet(1) == 0) {
        _thread_pool->start(_thread_init_callback);
        assert(!_acceptor->listenning());
        _loop->runInLoop(
            std::bind(&Acceptor::listen, get_pointer(_acceptor))
        );
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peer_addr) {
    _loop->assertInLoopThread();
    EventLoop* io_loop = _thread_pool->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", _ip_port.c_str(), _next_conn_id);
    ++_next_conn_id;
    string connect_name = _name + buf;

    LOG_INFO << "TcpServer::newConnection [" << _name
             << "] - new connection [" << connect_name 
             << "] from " << peer_addr.toIpPort();
    InetAddress local_addr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(
        io_loop, connect_name, sockfd, local_addr, peer_addr
    ));
    _connections[connect_name] = conn;
    conn->setConnectionCallback(_connection_callback);
    conn->setMessageCallback(_message_callback);
    conn->setWriteCompleteCallback(_write_complete_callback);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, _1)
    );
    io_loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    _loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    _loop->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << _name 
             << "] - connection " << conn->name();
    
    size_t n = _connections.erase(conn->name());
    (void)n;
    assert(n == 1);
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInloop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}