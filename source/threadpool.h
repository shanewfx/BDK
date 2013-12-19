#ifndef BASE_DEV_KIT_THREAD_POOL_H
#define BASE_DEV_KIT_THREAD_POOL_H

#include <boost\noncopyable.hpp>
#include <boost\function.hpp>
#include <string>
#include <deque>
#include <vector>

#include "thread.h"
#include "condition.h"
#include "mutex.h"

namespace BDK {

class thread_pool : boost::noncopyable
{
public:
    typedef boost::function<void ()> task_t;

    explicit thread_pool(const std::string& name = std::string());
   ~thread_pool();

    bool start(int thread_nums);
    void stop();

    void do_task(const task_t& task);

private:
    void run_in_thread();
    task_t get_task();

    bool               m_exit;
    std::deque<task_t> m_task_queue;
    std::vector<thread *> m_thread_pool;  
    std::string        m_name;

    condition          m_cond;
    mutex_lock         m_mutex;
};

}


#endif//BASE_DEV_KIT_THREAD_POOL_H