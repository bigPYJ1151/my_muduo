#ifndef MUDUO_BASE_SINGLETON_H
#define MUDUO_BASE_SINGLETON_H

#include <muduo/base/noncopyable.h>

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

namespace muduo {
namespace detial {
template <typename T>
struct has_no_destroy {
    template <typename C> static char test(decltype(&C::no_destroy));
    template <typename C> static int32_t test(...);
    const static bool value = sizeof(test<T>(0)) == 1;
};
}

template <typename T>
class Singleton : noncopyable {
public:
    Singleton() = delete;
    ~Singleton() = delete;

    static T& instance() {
        pthread_once(&_ponce, &Singleton::init);
        assert(_value != NULL);
        return *_value;
    }

private:
    static void init() {
        _value = new T();
        if (!detial::has_no_destroy<T>::value) {
            ::atexit(destroy);
        }
    }

    static void destroy() {
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;

        delete _value;
        _value = NULL;        
    }

    static pthread_once_t _ponce;
    static T* _value;
};

template <typename T>
pthread_once_t Singleton<T>::_ponce = PTHREAD_ONCE_INIT;

template <typename T>
T* Singleton<T>::_value = NULL;

}

#endif