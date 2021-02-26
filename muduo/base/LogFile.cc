
#include <muduo/base/LogFile.h>
#include <muduo/base/FileUtil.h>
#include <muduo/base/ProcessInfo.h>

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace muduo;

LogFile::LogFile(const string& basename,
                off_t roll_size,
                bool thread_safe,
                int flush_interval,
                int check_interval
                ) :
                _basename(basename),
                _roll_size(roll_size),
                _flush_interval(flush_interval),
                _check_every_n(check_interval),
                _count(0),
                _mutex(thread_safe ? new MutexLock : NULL),
                _start_of_period(0),
                _last_roll(0),
                _last_flush(0) {
    assert(basename.find('/') == string::npos);
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len) {
    if (_mutex) {
        MutexLockGuard lock(*_mutex);
        append_unlocked(logline, len);
    }
    else {
        append_unlocked(logline, len);
    }
}

void LogFile::flush() {
    if (_mutex) {
        MutexLockGuard lock(*_mutex);
        _file->flush();
    }
    else {
        _file->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len) {
    _file->append(logline, len);
    if (_file->writtenBytes() > _roll_size) {
        rollFile();
    }
    else {
        ++_count;
        if (_count > _check_every_n) {
            _count = 0;
            time_t now = ::time(NULL);
            time_t this_period = now / _k_roll_per_seconds * _k_roll_per_seconds;
            if (this_period != _start_of_period) {
                rollFile();
            }
            else if (now - _last_flush > _flush_interval) {
                _last_flush = now;
                _file->flush();
            }
        }
    }
}

bool LogFile::rollFile() {
    time_t now = 0;
    string filename = getLogFileName(_basename, &now);
    time_t start = now / _k_roll_per_seconds * _k_roll_per_seconds;

    if (now > _last_roll) {
        _last_roll = now;
        _last_flush = now;
        _start_of_period = start;
        _file.reset(new FileUtil::AppendFile(filename));
        return true;
    }
    return false;
}

string LogFile::getLogFileName(const string& basename, time_t* now) {
    string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm); // FIXME: localtime_r ?
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;

    filename += ProcessInfo::hostname();

    char pidbuf[32];
    snprintf(pidbuf, sizeof(pidbuf), ".%d", ProcessInfo::pid());
    filename += pidbuf;

    filename += ".log";

    return filename;
}

