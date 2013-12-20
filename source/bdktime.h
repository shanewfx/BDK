#ifndef BASE_DEV_KIT_TIME_H
#define BASE_DEV_KIT_TIME_H

#include "stdint.h"
#include <string>

namespace BDK {

//timestamp in local time, in microseconds resolution.
class timestamp 
{
public:
    timestamp() 
        : m_microSecondsSinceEpoch(0)
    { }

    explicit timestamp(int64_t microSecondsSinceEpoch)
        : m_microSecondsSinceEpoch(microSecondsSinceEpoch)
    { }

   ~timestamp()
    { }

    bool    isValid() const { return m_microSecondsSinceEpoch > 0; }
    int64_t microSecondsSinceEpoch() const { return m_microSecondsSinceEpoch; }

    std::string toString() const;
    std::string toFormattedString() const;


    static timestamp now();
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t m_microSecondsSinceEpoch;
};


inline bool operator<(timestamp lhs, timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(timestamp lhs, timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}


//Gets time difference of two timestamps, result in seconds.
//return (high-low) in seconds
inline double timeDifference(timestamp high, timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / timestamp::kMicroSecondsPerSecond;
}

inline timestamp addTime(timestamp time, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * timestamp::kMicroSecondsPerSecond);
    return timestamp(time.microSecondsSinceEpoch() + delta);
}

}

#endif//BASE_DEV_KIT_TIME_H