#ifndef NETRPC_NET_IO_THREAD_H
#define NETRPC_NET_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include <thread>
#include <memory>
#include <functional>
#include "netrpc/net/eventloop.h"

namespace netrpc {

class IOThread {
public:
    using ThreadFunc = std::function<void()>;

    IOThread();

    ~IOThread();

    EventLoop* getEventLoop();

    void start();

    void join();

public:
    static void* Main(void* arg);

private:
    
    pid_t m_thread_id {-1}; // 线程号
    pthread_t m_thread {0};

    EventLoop* m_eventloop {NULL}; // 当前 io 线程的 loop 对象

    sem_t m_init_semaphore;

    sem_t m_start_semaphore;
};

}

#endif