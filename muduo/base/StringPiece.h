#ifndef MUDUO_BASE_STRINGPIECE_H
#define MUDUO_BASE_STRINGPIECE_H

#include <muduo/base/Types.h>

#include <string_view>

namespace muduo {

// Passing C-style string argument
class StringArg {
public:
    StringArg(const char* str) : _str(str) {}

    StringArg(const string& str) : _str(str.c_str()) {}

    const char* c_str() const {
        return _str;
    }

private:
    const char* _str;
};

typedef std::string_view StringPiece;

}

#endif