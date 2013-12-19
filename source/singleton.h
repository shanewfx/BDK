#ifndef BASE_DEV_KIT_SINGLETON_H
#define BASE_DEV_KIT_SINGLETON_H

#include <boost\noncopyable.hpp>

#ifdef PTHREAD
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#else
#include "mutex.h"
#endif

namespace BDK {

template<typename T>
class singleton : boost::noncopyable
{
public:
    static T& get_instance()
    {
#ifdef PTHREAD
        pthread_once(&m_ponce, &singleton::init);
#else
        mutex_lock_guard lock(m_mutex);
        static T instance;
        m_singleton_instance = &instance;
#endif
        return *m_singleton_instance;
    }

private:
    singleton() {}
   ~singleton() {}

#ifdef PTHREAD
    static void init()
    {
        m_singleton_instance = new T();
        atexit(destroy);//!!
    }

    static void destroy()
    {
        delete m_singleton_instance;
    }

    static pthread_once_t m_ponce;

#else

#if 0
    class deleter
    {
    public:
        deleter() { }

        ~deleter()
        {
            if (singleton::m_singleton_instance != NULL) {
                printf("delete singleton\n");
                delete singleton::m_singleton_instance;
            }
        }
    };
    static deleter        m_deleter;
#else
    static mutex_lock     m_mutex;
#endif

#endif

    static T*             m_singleton_instance;
};

#ifdef PTHREAD
template<typename T>
pthread_once_t singleton<T>::m_ponce = PTHREAD_ONCE_INIT;

template<typename T>
T* singleton<T>::m_singleton_instance = NULL;

#else

#if 0
template<typename T>
typename singleton<T>::deleter singleton<T>::m_deleter;

template<typename T>
T* singleton<T>::m_singleton_instance = new T();

#else
template<typename T>
mutex_lock singleton<T>::m_mutex;

template<typename T>
T* singleton<T>::m_singleton_instance = NULL;
#endif

#endif

}

#endif//BASE_DEV_KIT_SINGLETON_H