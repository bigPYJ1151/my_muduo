
#ifndef WEB_SERVER_RESPONSE_H
#define WEB_SERVER_RESPONSE_H

#include <unordered_map>
#include <muduo/net/Buffer.h>

class Response {
public:
    using Buffer = muduo::net::Buffer;
    enum StatusCode {
        Unknow,
        S200Ok = 200,
        S301MovedPermanently = 301,
        S400BadRequest = 400,
        S404NotFound = 404,
    };

    explicit Response(bool close) : status_code_(Unknow), close_connection_(close) {}

    void setStatusCode(StatusCode code) {
        status_code_ = code;
    }

    void setStatusMessage(const std::string& message) {
        status_message_ = message;
    }

    void setCloseConnection(bool on) {
        close_connection_ = on;
    }

    bool closeConnection() const {
        return close_connection_;
    }

    void setContentType(const std::string& content_type) {
        addHeader("Content-Type", content_type);
    }

    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    void setBody(const std::string& body) {
        body_ = body;
    }

    void appendToBuffer(Buffer* output) const;

private:
    std::unordered_map<std::string, std::string> headers_;
    StatusCode status_code_;
    std::string status_message_;
    bool close_connection_;
    std::string body_;
};

#endif