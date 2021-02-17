#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H

#include <muduo/base/copyable.h>
#include <muduo/base/Types.h>

namespace muduo {
    
    // Time stamp in UTC with microseconds resolution.
    // Immutable.
    class Timestamp : public muduo::copyable {
    public:
        Timestamp() : _micro_second_since_epoch(0) {}

        /// @param micro_second_since_epoch_arg
        explicit Timestamp(int64_t micro_second_since_epoch_arg) : _micro_second_since_epoch(micro_second_since_epoch_arg) {}

        void swap(Timestamp& rhv) {
            std::swap(_micro_second_since_epoch, rhv._micro_second_since_epoch);
        }

        string toString() const;
        string toFormattedString(bool is_show_microseconds = true) const;

        bool valid() const {
            return _micro_second_since_epoch > 0;
        }

        int64_t microSecondsSinceEpoch() {
            return _micro_second_since_epoch;
        }

        time_t secondsSinceEpoch() {
            return static_cast<time_t>(_micro_second_since_epoch / kMicroSecondsPerSecond);
        }

        static Timestamp now();
        static Timestamp invalid() {
            return Timestamp();
        }

        static Timestamp fromUnixTime(time_t t, int microseconds = 0) {
            return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
        }

        static const int kMicroSecondsPerSecond = 1000000;
    
    private:
        int64_t _micro_second_since_epoch;
    };

    inline bool operator<(Timestamp lhs, Timestamp rhs) {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs) {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }
    
    /// @param high, low
    /// @return (high - low) represented by double
    inline double timeDifference(Timestamp high, Timestamp low) {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    /// @param time, seconds
    /// @return (time + seconds) represented by Timestamp
    inline Timestamp addTime(Timestamp time, double seconds) {
        int64_t diff = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);

        return Timestamp(time.microSecondsSinceEpoch() + diff); 
    }
}

#endif