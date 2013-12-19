#ifndef BASE_DEV_KIT_MUTEX_H
#define BASE_DEV_KIT_MUTEX_H

#include <boost\noncopyable.hpp>
#include <Windows.h>

#ifdef PTHREAD
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#endif

namespace BDK {

class mutex_lock : boost::noncopyable
{
public:
    mutex_lock()
    {
#ifdef PTHREAD

#if 0
        pthread_mutexattr_t attr; 
        pthread_mutexattr_init(&attr); 
        // …Ë÷√ recursive  Ù–‘
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP); 
        pthread_mutex_init(&m_mutex, &attr);
#else
        pthread_mutex_init(&m_mutex, NULL);
#endif

#else
        InitializeCriticalSection(&m_mutex);
#endif
    }

   ~mutex_lock()
    {
#ifdef PTHREAD
        pthread_mutex_destroy(&m_mutex);
#else
        DeleteCriticalSection(&m_mutex);
#endif
    }

    void lock()
    {
#ifdef PTHREAD
        pthread_mutex_lock(&m_mutex);
#else
        EnterCriticalSection(&m_mutex);
#endif
    }

    void unlock()
    {
#ifdef PTHREAD
        pthread_mutex_unlock(&m_mutex);
#else
        LeaveCriticalSection(&m_mutex);
#endif
    }

#ifdef PTHREAD
    pthread_mutex_t* get_pthread_mutex() /* non-const */
    {
        return &m_mutex;
    }
#endif

private:
#ifdef PTHREAD
    pthread_mutex_t  m_mutex;
#else
    CRITICAL_SECTION m_mutex;
#endif
};

class mutex_lock_guard : boost::noncopyable
{
public:
    explicit mutex_lock_guard(mutex_lock& lock) : m_lock(lock)
    {
        //printf("[mutex_lock_guard] lock\n");
        m_lock.lock();
    }

   ~mutex_lock_guard()
    {
        m_lock.unlock();
        //printf("[mutex_lock_guard] unlock\n");
    }

private:
    mutex_lock& m_lock;
};

}

#endif//BASE_DEV_KIT_MUTEX_H