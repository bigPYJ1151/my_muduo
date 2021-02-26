#ifndef MUDUO_BASE_LOGSTREAM_H
#define MUDUO_BASE_LOGSTREAM_H

#include <muduo/base/noncopyable.h>
#include <muduo/base/StringPiece.h>
#include <muduo/base/Types.h>

#include <assert.h>
#include <string.h>

namespace muduo {
namespace detial {

const int k_small_buffer = 4000;
const int k_larger_buffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : noncopyable {
public:
    FixedBuffer() : _cur(_data) {
        setCookie(cookieStart);
    }

    ~FixedBuffer() {
        setCookie(cookieEnd);
    }

    void append(const char* buf, size_t len) {
        if (implicit_cast<size_t>(avail()) > len) {
            memcpy(_cur, buf, len);
            _cur += len;
        }
    }

    const char* data() const {
        return _data;
    }

    int length() const {
        return static_cast<int>(_cur - _data);
    }

    char* current() {
        return _cur;
    }

    int avail() const {
        return static_cast<int>(end() - _cur);
    }

    void add(size_t len) {
        _cur += len;
    }

    void reset() {
        _cur = _data;
    }

    void bzero() {
        memZero(_data, sizeof(_data));
    }

    const char* debugString();

    void setCookie(void (*cookie)()) {
        _cookie = cookie;
    }

    string toString() const {
        return string(_data, length());
    }

    StringPiece toStringPiece() const {
        return StringPiece(_data, length());
    }

private:
    const char* end() const {
        return _data + sizeof(_data);
    }
    static void cookieStart();
    static void cookieEnd();

    void (*_cookie)();
    char _data[SIZE];
    char* _cur;
};
}

class LogStream : noncopyable {
    typedef LogStream self;
public:
    typedef detial::FixedBuffer<detial::k_small_buffer> Buffer;

    self& operator<<(bool v) {
        _buffer.append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);

    self& operator<<(const void*);
    
    self& operator<<(double);
    self& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }

    self& operator<<(char v) {
        _buffer.append(&v, 1);
        return *this;
    }

    self& operator<<(const char* str) {
        if (str) {
            _buffer.append(str, strlen(str));
        }
        else {
            _buffer.append("(null)", 6);
        }

        return *this;
    }

    self& operator<<(const unsigned char* str) {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    self& operator<<(const string& v) {
        _buffer.append(v.c_str(), v.size());
        return *this;
    }

    self& operator<<(const StringPiece& v) {
        _buffer.append(v.data(), v.size());
        return *this;
    }

    self& operator<<(const Buffer& v) {
        *this << v.toStringPiece();
        return *this;
    }

    void append(const char* data, int len) {
        _buffer.append(data, len);
    }

    const Buffer& buffer() const {
        return _buffer;
    }

    void resetBuffer() {
        _buffer.reset();
    }

private:
    void staticCheck();

    template <typename T>
    void formatInteger(T);

    Buffer _buffer;
    static const int k_max_number_size = 32;
};

class Fmt {
public:
    template <typename T>
    Fmt(const char* fmt, T val);

    const char* data() const {
        return _buf;
    }

    int length() const {
        return _length;
    }

private:
    char _buf[32];
    int _length;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
    s.append(fmt.data(), fmt.length());
    return s;
}

}

#endif