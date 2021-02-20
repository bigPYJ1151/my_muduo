
#include <muduo/base/Condition.h>

#include <errno.h>

bool muduo::Condition::waitForSeconds(double seconds) {
    struct timespec abstime;
    clock_gettime(CLOCK_MONOTONIC, &abstime);

    const int64_t k_nano_second_per_second = 1000000000;
    int64_t nano_seconds = static_cast<int64_t>(seconds * k_nano_second_per_second);

    abstime.tv_sec +=  static_cast<time_t>((abstime.tv_nsec + nano_seconds) / k_nano_second_per_second);
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nano_seconds) % k_nano_second_per_second);

    MutexLock::UnassignGuard ug(_mutex);

    return ETIMEDOUT == pthread_cond_timedwait(&_pcond, _mutex.getPthreadMutex(), &abstime);
}