
#include <web_server/Server.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <map>

int main(int argc, char* argv[]) {
    int num_threads = 0;
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }
    
    std::string image_path;
    image_path += "./image.jpg";

    muduo::net::EventLoop loop;
    Server server(&loop, InetAddress(8000), "Naive Server", false, image_path);
    server.setThreadNum(num_threads);
    server.start();
    loop.loop();
}