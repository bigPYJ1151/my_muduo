#include <muduo/base/Exception.h>
#include <muduo/base/CurrentThread.h>

namespace muduo {
    Exception::Exception(string msg) : _message(std::move(msg)), _stack(CurrentThread::stackTrace(false)) {}
}