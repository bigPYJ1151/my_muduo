
#include <muduo/base/ThreadPool.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>

#include <functional>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <endian.h>


struct sockaddr_in listen_addr;
int uniq_sock;

const struct sockaddr* addrCast(const struct sockaddr_in* addr) {
    return static_cast<const struct sockaddr*>(reinterpret_cast<const void*>(addr));
}

void threadFunc1() {
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN;
    ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, uniq_sock, &event);

    struct epoll_event ret_event;
    int num = 0;
    int pre = -1;
    LOG_INFO << reinterpret_cast<int64_t>(&epoll_fd) << " start!";
    while (true) {
        int num_events = ::epoll_wait(epoll_fd, &ret_event, 1, -1);

        LOG_INFO << reinterpret_cast<int64_t>(&epoll_fd) << " activate! event_num:" << num_events << " event_type: " << ret_event.events;
        struct ::sockaddr peer;
        socklen_t peer_len = sizeof(peer);
        int curr_fd = ::accept4(uniq_sock, &peer, &peer_len, SOCK_CLOEXEC);
        LOG_INFO << reinterpret_cast<int64_t>(&epoll_fd) << " accept 1 connection," << "fd=" << curr_fd << " current num: " << num;
        ++num;
        if (pre != -1) {
            ::close(pre);
            pre = curr_fd;
        }
    }
}

void threadFunc2() {
    int sock_fd = socket(listen_addr.sin_family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sock_fd < 0) {
        LOG_INFO << "socket created failed";
    }
    int opt = 1;
    ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));
    ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));
    ::setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));
    ::bind(
        sock_fd,
        addrCast(&listen_addr),
        sizeof(listen_addr)
    );
    ::listen(sock_fd, SOMAXCONN);

    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event);

    LOG_INFO << reinterpret_cast<int64_t>(&epoll_fd) << " start!";
    while (1) {
        struct epoll_event ret_event;
        int event_num = epoll_wait(epoll_fd, &ret_event, 1, -1);
        
        LOG_INFO << reinterpret_cast<int64_t>(&epoll_fd) << " activate! event_num:" << event_num << " event_type: " << ret_event.events;
        struct ::sockaddr peer;
        socklen_t peer_len = sizeof(peer);
        int curr_fd = ::accept4(sock_fd, &peer, &peer_len, SOCK_CLOEXEC);
        LOG_INFO << reinterpret_cast<int64_t>(&epoll_fd) << " accept 1 connection," << "fd=" << curr_fd;
    }
}   

int main() {

    ::memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htobe32(INADDR_ANY);
    listen_addr.sin_port = htobe16(static_cast<uint16_t>(2000));
    
    // uniq_sock = socket(listen_addr.sin_family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    // if (uniq_sock < 0) {
    //     LOG_SYSFATAL << "sockets created failed";
    // }
    // int opt = 1;
    // ::setsockopt(uniq_sock, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));
    // ::setsockopt(uniq_sock, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));
    // ::setsockopt(uniq_sock, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));
    // ::bind(
    //     uniq_sock, 
    //     addrCast(&listen_addr),
    //     sizeof(listen_addr));

    muduo::Thread t1(threadFunc2), t2(threadFunc2), t3(threadFunc2);

    sleep(2);
    
    // int ret = ::listen(uniq_sock, SOMAXCONN);
    // LOG_INFO << "start listening: " << ret;
    // pool.start(5);
    t1.start();
    t2.start();
    t3.start();
    t1.join();
}