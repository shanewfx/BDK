#include "thread.h"
#include <assert.h>

using namespace BDK;


thread::thread(const thread_func_ptr& func, const std::string& name)
: m_tid(0)
#ifndef PTHREAD
, m_thread(NULL)
#endif
, m_func(func)
, m_started(false)
, m_name(name)
{
#ifdef PTHREAD
    memset(&m_thread, 0, sizeof(pthread_t));
#endif
}

thread::~thread()
{
#ifndef PTHREAD
    if (m_thread) {
        CloseHandle(m_thread);
        m_thread = NULL;
    }
#endif
}

bool thread::start()
{
    assert(!m_started);
#ifdef PTHREAD
    pthread_create(&m_thread, NULL, &thread::entry_thread, this);
#else
    m_thread = CreateThread(NULL, 0, &thread::entry_thread, this, 0, &m_tid);
    if (NULL == m_thread) {
        return false;
    }
#endif
    m_started = true;
    return true;
}

bool thread::join()
{
    assert(m_started);
#ifdef PTHREAD
    pthread_join(m_thread, NULL);
#else
    WaitForSingleObject(m_thread, INFINITE);
    CloseHandle(m_thread);
    m_thread = NULL;
#endif
    m_started = false;
    return true;
}

#ifdef PTHREAD
void* __cdecl thread::entry_thread(void* context)
{
    assert(context != NULL);
    thread* t = static_cast<thread*>(context);
    t->m_tid = GetCurrentThreadId();
    t->m_func();
    return NULL;
}
#else
unsigned long __stdcall thread::entry_thread(void* context)
{
    assert(context != NULL);
    thread* t = static_cast<thread*>(context);
    t->m_func();
    return 0;
}
#endif