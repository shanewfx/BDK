#include <WinSock2.h> //included firstly, otherwise compile error

#include <stdio.h>
#include <tchar.h>
#include <signal.h>		// to catch Ctrl-C

#include <boost\bind.hpp>

#include "\BDK\source\thread.h"
using BDK::thread;

#include "\BDK\source\threadpool.h"
using BDK::thread_pool;

#include "\BDK\source\singleton.h"
using BDK::singleton;

#ifdef _DEBUG
#pragma comment(lib, "\\BDK\\bin\\BDK_d.lib")
#else
#pragma comment(lib, "\\BDK\\bin\\BDK.lib")
#endif

void thread_func(int id)
{
    static int a = singleton<int>::get_instance();
    printf("&a : %p\n", &a);

    int loop = 10;
    while (loop > 0) {
        printf("in thread %d, loop: %d\n", id, loop);
        --loop;
        Sleep(100);
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
#if 0
    thread t1(boost::bind(thread_func, 1), "thread1");
    thread t2(boost::bind(thread_func, 2), "thread2");

    t1.start();
    printf("t1 : %s: %d [t1: %d, t2: %d]\n", t1.name().c_str(), t1.tid(), t1.is_started(), t2.is_started());
    t2.start();
    printf("t2 : %s: %d [t1: %d, t2: %d]\n", t2.name().c_str(), t2.tid(), t1.is_started(), t2.is_started());

    t1.join();
    t2.join();
#endif

    thread_pool pool("test pool");
    pool.start(10);
    for (int i = 0; i < 10; i++) {
        thread_pool::task_t task(boost::bind(thread_func, i));
        pool.do_task(task);
    }

    printf("*** wait 5s begin ***\n");
    Sleep(5000);
    printf("*** wait 5s end ***\n");

    thread_pool::task_t task(boost::bind(thread_func, 1000));
    pool.do_task(task);

    printf("*** wait 5s begin ***\n");
    Sleep(5000);
    printf("*** wait 5s end ***\n");

    printf("== pool stop begin ==\n");
    pool.stop();
    printf("== pool stop end ==\n");

	return 0;
}

