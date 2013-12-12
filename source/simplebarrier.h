#ifndef BASE_DEV_KIT_SIMPLE_BARRIER_H
#define BASE_DEV_KIT_SIMPLE_BARRIER_H

#include <boost\noncopyable.hpp>
#include "condition.h"

namespace BDK {

class simple_barrier : boost::noncopyable
{
public:
    explicit simple_barrier(int count) 
        : m_count(count)
        , m_mutex()
        , m_cond(m_mutex)
    {

    }

    ~simple_barrier() {}

    void count_down()
    {
       mutex_lock_guard lock(m_mutex);
       --m_count;
       if (0 == m_count) {
           m_cond.notify(true);
       }
    }

    void wait()
    {
        mutex_lock_guard lock(m_mutex);
        if (m_count > 0) {
            m_cond.wait();
        }
    }

    int get_count() const 
    { 
        mutex_lock_guard lock(m_mutex);
        return m_count; 
    }

private:
    int m_count;

    mutex_lock m_mutex;
    condition  m_cond;
};

}

#endif//BASE_DEV_KIT_SIMPLE_BARRIER_H