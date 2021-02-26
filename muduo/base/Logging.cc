
#include <muduo/base/Logging.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/TimeZone.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

namespace muduo {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_last_second;

const char* strerror_tl(int saved_errno) {
    return strerror_r(saved_errno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::LogLevel initLogLevel() {
    if (::getenv("MUDUO_LOG_TRACE")) {
        return Logger::TRACE;
    }
    else if (::getenv("MUDUO_LOG_DEBUG")) {
        return Logger::DEBUG;
    }
    else {
        return Logger::INFO;
    }
}

Logger::LogLevel global_log_level = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

class T {
    public:
        T(const char* str, unsigned len) : _str(str), _len(len) {
            assert(strlen(str) == len);
        }

        const char* _str;
        const unsigned _len;
};

inline LogStream& operator<<(LogStream& s, T v) {
    s.append(v._str, v._len);
    return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
    s.append(v._data, v._size);
    return s;
}

void defaultOutput(const char* msg, int len) {
    fwrite(msg, 1, len, stdout);
}

void defaultFlush() {
    fflush(stdout);
}

Logger::OutputFunc global_output = defaultOutput;
Logger::FlushFunc global_flush = defaultFlush;
TimeZone global_log_timezone;

}

using namespace muduo;

Logger::Impl::Impl(LogLevel level, int saved_errno, const SourceFile& file, int line)
    : _time(Timestamp::now()), 
      _stream(),
      _level(level),
      _line(line),
      _basename(file) {
    formatTime();
    CurrentThread::tid();
    _stream << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
    _stream << T(LogLevelName[level], 6);
    if (saved_errno != 0) {
        _stream << strerror_tl(saved_errno) << " (errno=" << saved_errno << ") ";
    }
}

void Logger::Impl::formatTime() {
    int64_t microSecondsSinceEpoch = _time.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_last_second)
    {
        t_last_second = seconds;
        struct tm tm_time;
        if (global_log_timezone.valid())
        {
            tm_time = global_log_timezone.toLocalTime(seconds);
        }
        else
        {
            ::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
        }

        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17); (void)len;
    }

    if (global_log_timezone.valid())
    {
        Fmt us(".%06d ", microseconds);
        assert(us.length() == 8);
        _stream << T(t_time, 17) << T(us.data(), 8);
    }
    else
    {
        Fmt us(".%06dZ ", microseconds);
        assert(us.length() == 9);
        _stream << T(t_time, 17) << T(us.data(), 9);
    }
}

void Logger::Impl::finish() {
    _stream << " - " << _basename << ':' << _line << '\n';
}

Logger::Logger(SourceFile file, int line) : _impl(INFO, 0, file, line) {}
Logger::Logger(SourceFile file, int line, LogLevel level, const char* func) : _impl(level, 0, file, line) {
    _impl._stream << func << ' ';
}


Logger::Logger(SourceFile file, int line, LogLevel level) : _impl(level, 0, file, line) {}
Logger::Logger(SourceFile file, int line, bool to_abort) : _impl(to_abort ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
    _impl.finish();
    const LogStream::Buffer& buf(stream().buffer());
    global_output(buf.data(), buf.length());
    if (_impl._level == FATAL) {
        global_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level) {
    global_log_level = level;
}

void Logger::setOutput(OutputFunc out) {
    global_output = out;
}

void Logger::setFlush(FlushFunc flush) {
    global_flush = flush;
}

void Logger::setTimeZone(const TimeZone& tz) {
    global_log_timezone = tz;
}
