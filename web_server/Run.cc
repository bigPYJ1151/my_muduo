
#include <web_server/Server.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <map>

int main(int argc, char* argv[]) {
    int num_threads = 0;
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }
    muduo::net::EventLoop loop;
    Server server(&loop, InetAddress(8000), "Naive Server");
    server.setThreadNum(num_threads);
    server.start();
    loop.loop();
}