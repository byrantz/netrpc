#ifndef NETRPC_NET_EVENTLOOP_H
#define NETRPC_NET_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "netrpc/net/fd_event.h"
#include "netrpc/net/wakeup_fd_event.h"
#include <mutex>

namespace netrpc {

using WakeFdEventPtr = std::shared_ptr<WakeUpFdEvent>;
    
class EventLoop {
public:
    EventLoop();

    ~EventLoop();

    void loop();

    void wakeup();

    void stop();

    void addEpollEvent(FdEvent* event);

    void deleteEpollEvent(FdEvent* event);

    bool isInLoopThread(); 

    void addTask(std::function<void()> cb, bool is_wake_up = false);

private:
    void dealWakeup();

    void initWakeUpFdEvent();

private:
    pid_t m_thread_id {0};

    int m_epoll_fd {0};

    int m_wakeup_fd {0};

    WakeFdEventPtr m_wakeup_fd_event {NULL};

    bool m_quit {false};

    std::set<int> m_listen_fds;

    std::queue<std::function<void()>> m_pending_tasks;

    std::mutex m_mutex;
};

}

#endif