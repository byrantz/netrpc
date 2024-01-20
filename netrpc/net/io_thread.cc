#include <pthread.h>
#include <assert.h>
#include "netrpc/net/io_thread.h"
#include "netrpc/common/log.h"
#include "netrpc/common/util.h"

namespace netrpc {

IOThread::IOThread() {

    int rt = sem_init(&m_init_semaphore, 0, 0);
    assert(rt == 0);

    rt = sem_init(&m_start_semaphore, 0, 0);
    assert(rt == 0);

    pthread_create(&m_thread, NULL, &IOThread::Main, this);

    // wait, 直到新线程执行完 Main 函数的前置
    sem_wait(&m_init_semaphore);

    DEBUGLOG("IOThread [%d] create success", m_thread_id);
}

IOThread::~IOThread() {
    m_eventloop->stop();
    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);

    // 等待当前 IO 线程结束 
    pthread_join(m_thread, NULL);

    if (m_eventloop) {
        delete m_eventloop;
        m_eventloop = NULL;
    }
}

void* IOThread::Main(void* arg) {
    IOThread* thread = static_cast<IOThread*> (arg);

    thread->m_eventloop = new EventLoop();
    thread->m_thread_id = getThreadId();

    // 唤醒等待的线程
    sem_post(&thread->m_init_semaphore);

    // 让 IO 线程等待，直到我们主动启动
    // 静态成员函数必须通过对象才能访问非静态成员变量
    DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);

    sem_wait(&thread->m_start_semaphore);
    DEBUGLOG("IOThread %d start loop", thread->m_thread_id);
    thread->m_eventloop->loop();

    DEBUGLOG("IOThread %d end loop", thread->m_thread_id);

    return NULL;
}

EventLoop* IOThread::getEventLoop() {
    return m_eventloop;
}

void IOThread::start() {
    DEBUGLOG("Now invoke IOThread %d", m_thread_id);
    sem_post(&m_start_semaphore);
}

void IOThread::join() {
    pthread_join(m_thread, NULL);
}

}