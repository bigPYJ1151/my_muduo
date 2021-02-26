#ifndef MUDUO_BASE_LOGGING_H
#define MUDUO_BASE_LOGGING_H

#include <muduo/base/LogStream.h>
#include <muduo/base/Timestamp.h>

namespace muduo {
class TimeZone;

class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    class SourceFile {
    public:
        template <int N>
        SourceFile(const char (&arr)[N]) : _data(arr), _size(N) {
            const char* slash = strrchr(_data, '/');
            if (slash) {
                _data = slash + 1;
                _size -= static_cast<int>(_data - arr);
            }
        }

        explicit SourceFile(const char* filename) : _data(filename) {
            const char* slash = strchr(filename, '/');
            if (slash) {
                _data = slash + 1;
            }
            _size = static_cast<int>(strlen(_data));
        }

        const char* _data;
        int _size;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool to_abort);
    ~Logger();

    LogStream& stream() {
        return _impl._stream;
    }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    static void setTimeZone(const TimeZone& tz);



private:
    class Impl {
        public:
            typedef Logger::LogLevel LogLevel;
            Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
            void formatTime();
            void finish();

            Timestamp _time;
            LogStream _stream;
            LogLevel _level;
            int _line;
            SourceFile _basename;
    };

    Impl _impl;
};

extern Logger::LogLevel global_log_level;

inline Logger::LogLevel Logger::logLevel() {
    return global_log_level;
}

#define LOG_TRACE if (muduo::Logger::logLevel() <= muduo::Logger::TRACE) \
    muduo::Logger(__FILE__, __LINE__, muduo::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG) \
    muduo::Logger(__FILE__, __LINE__, muduo::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
    muduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN muduo::Logger(__FILE__, __LINE__, muduo::Logger::WARN).stream()
#define LOG_ERROR muduo::Logger(__FILE__, __LINE__, muduo::Logger::ERROR).stream()
#define LOG_FATAL muduo::Logger(__FILE__, __LINE__, muduo::Logger::FATAL).stream()
#define LOG_SYSERR muduo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int saved_errno);

#define CHECK_NOTNULL(val)\
    ::muduo::CheckNotNull(__File__, __LINE__, "'" #val "' Must be not NULL", (val));

template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr) {
    if (ptr == NULL) {
        Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}
}

#endif