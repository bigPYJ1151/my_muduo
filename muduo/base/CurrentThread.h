#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include <muduo/base/Types.h>

namespace muduo {
    namespace CurrentThread {
        extern __thread int t_cached_tid;
        extern __thread char t_tid_string[32];
        extern __thread int t_tid_string_len;
        extern __thread const char* t_thread_name;
        void cacheTid();

        inline int tid() {
            if (__builtin_expect(t_cached_tid == 0, 0)) {
                cacheTid();
            }
            return t_cached_tid;
        }

        inline const char* tidString() {
            return t_tid_string;
        }

        inline int tidStringLength() {
            return t_tid_string_len;
        }

        inline const char* name() {
            return t_thread_name;
        }

        bool isMainThread();

        void sleepUsec(int64_t usec);

        string stackTrace(bool demangle);
    }
}

#endif