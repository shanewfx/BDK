#ifndef _BASE_DEV_KIT_THREAD_H_
#define _BASE_DEV_KIT_THREAD_H_

#include <boost\noncopyable.hpp>
#include <boost\function.hpp>

#include <string>
#include <Windows.h>

#ifdef PTHREAD
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#endif

namespace BDK {

class thread : boost::noncopyable
{
public:
    typedef boost::function<void ()> thread_func_ptr;
//#ifndef PTHREAD
    typedef unsigned long pid_t;
//#endif

    explicit thread(const thread_func_ptr& func, const std::string& name = std::string());
   ~thread();

    bool  start();
    bool  join();

    bool  is_started() const { return m_started; }
    pid_t tid() const { return m_tid; }
    const std::string& name() const { return m_name; }

private:

#ifdef PTHREAD
    static void* __cdecl entry_thread(void* context);
    pthread_t       m_thread;
#else
    static unsigned long __stdcall entry_thread(void* context);
    HANDLE          m_thread;
#endif
    pid_t           m_tid;
    thread_func_ptr m_func;
    bool            m_started;

    std::string     m_name;
};


} //namespace basedevkit


#endif//_BASE_DEV_KIT_THREAD_H_