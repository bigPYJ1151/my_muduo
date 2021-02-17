
#include <muduo/base/Timestamp.h>

#include <sys/time.h>
#include <cstdio>
#include <cinttypes> 

using namespace muduo;

static_assert(sizeof(Timestamp) == sizeof(int64_t),
                "Timestamp is same size as int64_t");

string Timestamp::toString() const {
    char buffer[32] = {0};
    int64_t seconds = _micro_second_since_epoch / kMicroSecondsPerSecond;
    int64_t microseconds = _micro_second_since_epoch % kMicroSecondsPerSecond;
    snprintf(buffer, sizeof(buffer) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);

    return buffer;
}

string Timestamp::toFormattedString(bool showMicroseconds) const {
    char buffer[64] = {0};
    time_t seconds = static_cast<time_t>(_micro_second_since_epoch / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroseconds) {
        int microseconds = static_cast<int>(_micro_second_since_epoch % kMicroSecondsPerSecond);
        snprintf(buffer, sizeof(buffer), "%4d%02d%02d %02d:%02d:%02d.%06d", 
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
    }
    else {
        snprintf(buffer, sizeof(buffer), "%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }

    return buffer;
} 

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;

    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}