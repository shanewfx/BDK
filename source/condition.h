#ifndef BASE_DEV_KIT_CONDITION_H
#define BASE_DEV_KIT_CONDITION_H

#include <boost\noncopyable.hpp>
#include <Windows.h>

#include "mutex.h"

namespace BDK {

#ifdef PTHREAD

class condition : boost::noncopyable
{
public:
    condition(mutex_lock& mutex);
    ~condition();

    void wait();
    void notify(bool broadcast = false);

private:
    mutex_lock&    m_mutex;
    pthread_cond_t m_pcond;
};

#else

class condition : boost::noncopyable
{
public:
    condition();
    ~condition();
   
    void wait();
    void notify();

private:
    HANDLE m_event;
};

#endif

}

#endif//BASE_DEV_KIT_CONDITION_H