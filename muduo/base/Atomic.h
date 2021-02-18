#ifndef MUDUO_BASE_ATOMIC_H
#define MUDUO_BASE_ATOMIC_H

#include <muduo/base/noncopyable.h>
#include <cstdint>

namespace muduo {
    namespace detial {
        template <typename T>
        class AtomicIntegerT : noncopyable {
        public:
            AtomicIntegerT() : _value(0) {}

            // Atomic load
            T get() {
               return __atomic_load_n(&_value, __ATOMIC_SEQ_CST);
            } 

            // Atomic add, and return value before add
            T getAndAdd(T x) {
                return __atomic_fetch_add(&_value, x, __ATOMIC_SEQ_CST);
            }

            // Atomic add, and return result
            T addAndGet(T x) {
                return __atomic_add_fetch(&_value, x, __ATOMIC_SEQ_CST);
            }

            // ++i
            T incrementAndGet() {
                return addAndGet(1);
            }

            // i++
            T getAndIncrement() {
                return getAndAdd(1);
            }

            // --i
            T decrementAndGet() {
                return addAndGet(-1);
            }
            
            // i--
            T getAndDecrement() {
                return getAndAdd(-1);
            }

            void add(T x) {
                getAndAdd(x);
            }

            void increment() {
                incrementAndGet();
            }

            void decrement() {
                decrementAndGet();
            }

            // Set _value to new_value and return previous _value
            T getAndSet(T new_value) {
                return __atomic_exchange_n(&_value, new_value, __ATOMIC_SEQ_CST);
            }

        private:
            T _value;
        };
    }

    typedef detial::AtomicIntegerT<int32_t> AtomicInt32;
    typedef detial::AtomicIntegerT<int64_t> AtomicInt64;
}

#endif
