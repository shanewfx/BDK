#include "threadpool.h"
#include <boost\bind.hpp>

#include <stdio.h>

using namespace BDK;


thread_pool::thread_pool(const std::string& name)
: m_name(name)
, m_exit(false)
#ifdef PTHREAD
, m_cond(m_mutex)
#endif
{

}

thread_pool::~thread_pool()
{
    stop();
}

bool thread_pool::start(int thread_nums)
{
    assert(m_thread_pool.empty());
    m_thread_pool.reserve(thread_nums);
    for (int i = 0; i < thread_nums; i++) {
        char id[32];
        sprintf_s(id, sizeof id, "%d", i);
        m_thread_pool.push_back(
            new thread(boost::bind(&thread_pool::run_in_thread, this), m_name + id));
        m_thread_pool[i]->start();
    }
    return true;
}

void thread_pool::stop()
{
    m_exit = true;
#ifdef PTHREAD
    {
    printf("=== [stop] get lock begin ===\n");
    mutex_lock_guard lock(m_mutex);
    printf("=== [stop] get lock end ===\n");

    m_cond.notify(true);
    }
#endif
    for (int i = 0; i < m_thread_pool.size(); i++) {
#ifndef PTHREAD
        m_cond.notify();
#endif
        printf("join : %s [tid: %d] begin\n", m_thread_pool[i]->name().c_str(), m_thread_pool[i]->tid());
        m_thread_pool[i]->join();
        printf("join : %s [tid: %d]end\n", m_thread_pool[i]->name().c_str(), m_thread_pool[i]->tid());
    }

    for (int i = 0; i < m_thread_pool.size(); i++) {
        delete m_thread_pool[i];
    }
    m_thread_pool.clear();
    m_task_queue.clear();
}

void thread_pool::do_task(const task_t& task)
{
    if (m_thread_pool.empty()) {
        task();
    }
    else {
        printf("=== [do_task] get lock begin ===\n");
        mutex_lock_guard lock(m_mutex);
        printf("=== [do_task] get lock end ===\n");
        m_task_queue.push_back(task);
        m_cond.notify();
    }
}

thread_pool::task_t thread_pool::get_task()
{
#ifdef PTHREAD
    printf("=== get lock begin ===\n");
    mutex_lock_guard lock(m_mutex);
    printf("=== get lock end ===\n");
#endif

    while (m_task_queue.empty() && !m_exit) {
        printf("=== wait task begin ===\n");
        m_cond.wait();
        printf("=== wait task end ===\n");
    }

#ifndef PTHREAD
    printf("=== get lock begin ===\n");
    mutex_lock_guard lock(m_mutex);
    printf("=== get lock end ===\n");
#endif


    task_t task;
    if (!m_task_queue.empty()) {
        task = m_task_queue.front();
        m_task_queue.pop_front();
    }
    return task;
}

void thread_pool::run_in_thread()
{
    while (!m_exit) {
        task_t task(get_task());
        if (task) {
            printf("=== do task begin ===\n");
            task();
            printf("=== do task end ===\n");
        }
    }
    printf("=== exit thread ===\n");
}