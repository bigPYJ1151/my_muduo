#ifndef MUDUO_BASE_TYPES_H
#define MUDUO_BASE_TYPES_H

#include <cstdint>
#include <cstring>
#include <string>

#ifndef NDEBUG
#include <cassert>
#endif

namespace muduo {
using std::string;

inline void memZero(void* ptr, size_t n) {
    memset(ptr, 0, n);
}

template <typename To, typename From>
inline To implicit_cast(From const & f) {
    return f;
}
}

#endif