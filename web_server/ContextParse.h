#ifndef WEB_SERVER_CONTEXTPARSE_H
#define WEB_SERVER_CONTEXTPARSE_H

#include <muduo/net/Buffer.h>
#include <web_server/Request.h>

class ContextParse {
public:
    using Buffer = muduo::net::Buffer;
    using Timestamp = muduo::Timestamp;

    enum ContextParseState {
        ExceptRequestLine,
        ExceptHeader,
        ExceptBody,
        GotAll,
    };

    ContextParse() : state_(ExceptRequestLine) {}

    bool parseRequest(Buffer* buf, Timestamp receive_time);

    bool gotAll() const {
        return state_ == GotAll;
    }

    void reset() {
        state_ = ExceptRequestLine;
        Request dummy;
        request_.swap(dummy);
    }

    const Request& request() const {
        return request_;
    }

    Request& request() {
        return request_;
    }

private:
    bool processRequestLine(const char* begin, const char* end);
    
    ContextParseState state_;
    Request request_;
};

#endif
