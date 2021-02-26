
#include <muduo/base/LogStream.h>

#include <algorithm>
#include <limits>
#include <type_traits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::detial;

namespace muduo {
namespace detial {

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");
const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");

//Convert Integer to String
template <typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char *p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    }
    while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';

    std::reverse(buf, p);

    return p - buf;
}

size_t convertHex(char buf[], uintptr_t value) {
    uintptr_t i = value;
    char* p = buf;
    do {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    }
    while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

template class FixedBuffer<k_small_buffer>;
template class FixedBuffer<k_larger_buffer>;
}
}

template<int SIZE>
const char* FixedBuffer<SIZE>::debugString() {
  *_cur = '\0';
  return _data;
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieStart() {
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieEnd() {
}

void LogStream::staticCheck() {
  static_assert(k_max_number_size - 10 > std::numeric_limits<double>::digits10,
                "k_max_number_size is large enough");
  static_assert(k_max_number_size - 10 > std::numeric_limits<long double>::digits10,
                "k_max_number_size is large enough");
  static_assert(k_max_number_size - 10 > std::numeric_limits<long>::digits10,
                "k_max_number_size is large enough");
  static_assert(k_max_number_size - 10 > std::numeric_limits<long long>::digits10,
                "k_max_number_size is large enough");
}

template <typename T>
void LogStream::formatInteger(T v) {
    if (_buffer.avail() >= k_max_number_size) {
        size_t len = convert(_buffer.current(), v);
        _buffer.add(len);
    }
}

LogStream& LogStream::operator<<(short v)
{
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (_buffer.avail() >= k_max_number_size) {
        char* buf = _buffer.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf + 2, v);
        _buffer.add(len + 2);
    }

    return *this;
}

LogStream& LogStream::operator<<(double v)
{
  if (_buffer.avail() >= k_max_number_size)
  {
    int len = snprintf(_buffer.current(), k_max_number_size, "%.12g", v);
    _buffer.add(len);
  }
  return *this;
}

template <typename T>
Fmt::Fmt(const char* fmt, T val) {
    static_assert(std::is_arithmetic<T>::value == true, "must be arithmetic type.");
    _length = snprintf(_buf, sizeof(_buf), fmt, val);
    assert(static_cast<size_t>(_length) < sizeof(_buf));
}

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
