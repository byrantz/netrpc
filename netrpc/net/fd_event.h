#ifndef NETRPC_NET_FDEVENT_H
#define NETRPC_NET_FDEVENT_H

#include <functional>
#include <sys/epoll.h>

namespace netrpc {

class FdEvent {
public :
    enum TriggerEvent {
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT,
        ERROR_EVENT = EPOLLERR,
    };

    FdEvent(int fd);

    FdEvent();

    ~FdEvent();

    // 将对应的 sockfd 设置为非阻塞
    void setNonBlock();

    std::function<void()> handler(TriggerEvent event_type);

    void listen(TriggerEvent event_type, std::function<void()> callback, std::function<void()> error_callback = nullptr);

    // 取消监听
    void cancle(TriggerEvent event_type);

    int getFd() const {
        return m_fd;
    }

    epoll_event getEpollEvent() {
        return m_listen_events;
    }

protected:
    int m_fd {-1};

    epoll_event m_listen_events; 

    std::function<void()> m_read_callback {nullptr};
    std::function<void()> m_write_callback {nullptr};
    std::function<void()> m_error_callback {nullptr};
};

}

#endif