#include "condition.h"
#include <assert.h>

using namespace BDK;


#ifdef PTHREAD

condition::condition(mutex_lock& mutex)
: m_mutex(mutex)
{
    pthread_cond_init(&m_pcond, NULL);
}

condition::~condition()
{
    pthread_cond_destroy(&m_pcond);
}

void BDK::condition::wait()
{
    pthread_cond_wait(&m_pcond, m_mutex.get_pthread_mutex());
}

void BDK::condition::notify(bool broadcast)
{
    if (broadcast) {
        pthread_cond_broadcast(&m_pcond);
    }
    else {
        pthread_cond_signal(&m_pcond);
    }
}

#else

condition::condition()
{
    m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    assert(m_event != NULL);
}

condition::~condition()
{
    if (m_event) {
        CloseHandle(m_event);
        m_event = NULL;
    }
}

void BDK::condition::wait()
{
    WaitForSingleObject(m_event, INFINITE);
    ResetEvent(m_event);
}

void BDK::condition::notify()
{
    SetEvent(m_event);
}

#endif