#ifndef MUDUO_BASE_EXCEPTION_H
#define MUDUO_BASE_EXCEPTION_H

#include <muduo/base/Types.h>
#include <exception>

namespace muduo {
    class Exception : public std::exception {
    public:
        Exception(string what);
        ~Exception() noexcept override = default;

        const char* what() const noexcept override {
            return _message.c_str();
        }

        const char* stackTrace() const noexcept {
            return _stack.c_str();
        }

    private:
        string _message;
        string _stack;
    };
}

#endif
