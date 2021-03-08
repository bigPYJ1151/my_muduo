
#ifndef WEB_SERVER_REQUEST
#define WEB_SERVER_REQUEST

#include <muduo/base/Timestamp.h>

#include <assert.h>
#include <stdio.h>
#include <string>
#include <unordered_map>

class Request {
public:
    enum Method {
        Invalid, Get, Post, Head, Put, Delete
    };
    enum Version {
        Unknow, Http10, Http11
    };

    Request() : method_(Invalid), version_(Unknow) {}

    void setQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }

    const std::string& query() const {
        return query_;
    }

    void setReceiveTime(muduo::Timestamp t) {
        receive_time_ = t;
    }

    std::string getHeader(const std::string& field) const {
        std::string result;
        if (headers_.count(field)) {
            result = headers_.at(field);
        }
        return result;
    }

    const std::unordered_map<std::string, std::string>& headers() const {
        return headers_;
    }

    void swap(Request& rh) {
        std::swap(method_, rh.method_);
        std::swap(version_, rh.version_);
        path_.swap(rh.path_);
        query_.swap(rh.query_);
        receive_time_.swap(rh.receive_time_);
        headers_.swap(rh.headers_);
    }

    void addHeader(const char* start, const char* colon, const char* end) {
        std::string field(start, colon);
        ++colon;
        while (colon < end && isspace(*colon)) {
            ++colon;
        }
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size() - 1])) {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }

    void setPath(const char* start, const char* end) {
        path_.assign(start, end);
    }

    const std::string& path() const {
        return path_;
    }

    bool setMethod(const char* start, const char* end) {
        assert(method_ == Invalid);
        std::string method(start, end);
        if (method == "GET") {
            method_ = Get;
        }
        else if (method == "POST") {
            method_ = Post;
        }
        else if (method == "HEAD") {
            method_ = Head;
        }
        else if (method == "PUT") {
            method_ = Put;
        }
        else if (method == "DELETE") {
            method_ = Delete;
        }
        else {
            method_ = Invalid;
        }

        return method_ != Invalid;
    }

    const char* methodString() const {
        const char* result = "UNKNOW";
        switch (method_) {
            case Get:
                result = "GET";
                break;
            case Post:
                result = "POST";
                break;
            case Head:
                result = "HEAD";
                break;
            case Put:
                result = "PUT";
                break;
            case Delete:
                result = "DELETE";
                break;
            default:
                break;
        }

        return result;
    }

    void setVersion(Version v) {
        version_ = v;
    }

    Version getVersion() const {
        return version_;
    }


private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    muduo::Timestamp receive_time_;
    std::unordered_map<std::string, std::string> headers_;
};

#endif
