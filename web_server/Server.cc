
#include <web_server/Server.h>
#include <muduo/base/Logging.h>

static std::vector<unsigned char> image_;

void HttpRequest(const Request& req, Response* resp) {
    LOG_TRACE << "Headers " << req.methodString() << " " << req.path(); 
    LOG_INFO << req.path();
    if (req.path() == "/") {
        resp->setStatusCode(Response::S200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "Naive Server");
        std::string now = muduo::Timestamp::now().toFormattedString();
        resp->setBody(
        "<html><head><title>This is title</title></head>"
        "<body><h1>Hello</h1>Now is " + now +
        "</body></html>"
        );
    }
    else if (req.path() == "/image.jpg") {
        resp->setStatusCode(Response::S200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("image/jpg");
        resp->addHeader("Server", "Naive Server");
        std::string now = muduo::Timestamp::now().toFormattedString();
        resp->setBody(image_);
    }
    else if (req.path() == "/hello") {
        resp->setStatusCode(Response::S200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->addHeader("Server", "Naive Server");
        resp->setBody("Hello, world!\n");
    }
    else {
        resp->setStatusCode(Response::S404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

Server::Server(EventLoop* loop,
                const InetAddress& listen_addr,
                const std::string& name,
                bool use_timer,
                std::string image_path,
                TcpServer::Option option
                ) :
                server_(loop, listen_addr, name, option),
                http_callback_(HttpRequest),
                timer_(5, loop),
                use_timer_(use_timer)
                 {
    server_.setConnectionCallback(
        std::bind(&Server::onConnection, this, muduo::_1)
    );
    server_.setMessageCallback(
        std::bind(&Server::onMessage, this, muduo::_1, muduo::_2, muduo::_3)
    );

    if (use_timer_) {
        timer_.start();
    }

    if (!image_path.empty()) {
        FILE* fp = nullptr;
        fp = fopen(image_path.c_str(), "rb");
        assert(fp != nullptr);

        fseek(fp, 0, SEEK_END);
        long int file_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        image_.resize(file_len);

        fread(image_.data(), 1, file_len, fp);
        fclose(fp);
    }
}

void Server::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        TimeWheelEntryPtr context;
        if (use_timer_) {
            context = std::make_shared<TimeWheelEntry>(conn);
            timer_.insertEntry(context);
        }
        conn->setContext(ContextType(ContextParse(), std::weak_ptr(context)));
    }
}

void Server::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receive_time) {
    ContextType* context_pair = std::any_cast<ContextType>(conn->getMutableContext());
    ContextParse* context = &(context_pair->first);
    if (!context->parseRequest(buf, receive_time)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll()) {
        onRequest(conn, context->request());
        context->reset();
    }
}

void Server::onRequest(const TcpConnectionPtr& conn, const Request& req) {
    const std::string& connection = req.getHeader("Connection");
    bool is_short_link = connection == "close" || (req.getVersion() == Request::Http10 && connection != "Keep-Alive");
    Response response(is_short_link);
    http_callback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection()) {
        conn->shutdown();
    }
    else if (use_timer_) { 
        ContextType* context_pair = std::any_cast<ContextType>(conn->getMutableContext());
        weakTimeWheelEntryPtr* context = &(context_pair->second);
        if (context->lock()) {
            timer_.insertEntry(context->lock());
        }
    }
}

void Server::start() {
    LOG_WARN << "Server[" << server_.name() << "] starts listening on " << server_.ipPort();
    server_.start();
}

