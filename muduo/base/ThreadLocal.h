#ifndef MUDUO_BASE_THREADLOCAL_H
#define MUDUO_BASE_THREADLOCAL_H

#include <muduo/base/Mutex.h>
#include  <muduo/base/noncopyable.h>

#include <pthread.h>

namespace muduo {
template <typename T>
class ThreadLocal : noncopyable {
public:
    ThreadLocal() {
        MCHECK(pthread_key_create(&_pkey, &ThreadLocal::destructor));
    }

    ~ThreadLocal() {
        int res;
        MCHECK(res = pthread_key_delete(_pkey));
        printf("tid: %d, threadlocal destructor called, res: %d\n", CurrentThread::tid(), res);
    }

    T& value() {
        T* ans = static_cast<T*>(pthread_getspecific(_pkey));
        if (!ans) {
            T* new_obj = new T();
            MCHECK(pthread_setspecific(_pkey, new_obj));
            ans = new_obj;
        }

        return *ans;
    }

private:
    static void destructor(void* x) {
        T* obj = static_cast<T*>(x);
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        delete obj;
    }

    pthread_key_t _pkey;
};
}

#endif
