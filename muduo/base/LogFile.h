
#ifndef MUDUO_BASE_LOGFILE_H
#define MUDUO_BASE_LOGFILE_H

#include <muduo/base/Mutex.h>
#include <muduo/base/Types.h>

#include <memory>

namespace muduo {
namespace FileUtil {

class AppendFile;

}

class LogFile : noncopyable {
public:
    LogFile(const string& basename, off_t roll_size, bool thread_safe = true, int flush_interval = 3, int check_interval = 1024);
    ~LogFile();
    
    void append(const char* logline, int len);
    void flush();
    bool rollFile();
    
private:
    void append_unlocked(const char* logline, int len);
    static string getLogFileName(const string& basename, time_t* now);

    const string _basename;
    const off_t _roll_size;
    const int _flush_interval;
    const int _check_every_n;

    int _count;

    std::unique_ptr<MutexLock> _mutex;
    time_t _start_of_period;
    time_t _last_roll;
    time_t _last_flush;
    std::unique_ptr<FileUtil::AppendFile> _file;

    const static int _k_roll_per_seconds = 60 * 60 * 24;
};

}

#endif