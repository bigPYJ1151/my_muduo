#ifndef MUDUO_BASE_THREADLOCALSINGLETON_H
#define MUDUO_BASE_THREADLOCALSINGLETON_H

#include <muduo/base/noncopyable.h>

#include <assert.h>
#include <pthread.h>

namespace muduo {
template <typename T>
class ThreadLocalSingleton : noncopyable {
public:
    ThreadLocalSingleton() = delete;
    ~ThreadLocalSingleton() = delete;

    static T& instance() {
        if (!_t_value) {
            _t_value = new T();
            _deleter.set(_t_value);
        }

        return *_t_value;
    }
private:
    static void destructor(void* obj) {
        assert(obj == _t_value);
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        delete _t_value;
        _t_value = 0;
    }

    class Deleter {
    public:
        Deleter() {
            pthread_key_create(&_pkey, &ThreadLocalSingleton::destructor);
        }

        ~Deleter() {
            pthread_key_delete(_pkey);
        }

        void set(T* new_obj) {
            assert(pthread_getspecific(_pkey) == NULL);
            pthread_setspecific(_pkey, new_obj);
        }
        
        pthread_key_t _pkey;
    };

    static __thread T* _t_value;
    static Deleter _deleter;
};

template <typename T>
__thread T* ThreadLocalSingleton<T>::_t_value = 0;

template <typename T>
typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::_deleter;

}
#endif