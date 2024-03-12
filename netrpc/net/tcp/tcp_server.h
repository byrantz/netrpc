#ifndef NETRPC_NET_TCP_SERVER_H
#define NETRPC_NET_TCP_SERVER_H

#include <set>
#include "netrpc/net/tcp/tcp_acceptor.h"
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/net/io_thread_group.h"
#include "netrpc/net/tcp/tcp_connection.h"

namespace netrpc {

class TcpServer {
public:
    TcpServer(NetAddr::NetAddrPtr local_addr);

    ~TcpServer();

    void start();

    // 消除 closed 的连接
    void ClearClientTimerFunc();

private:
    void init();

    // 当有新客户端连接之后需要执行
    void onAccept();

private:
    TcpAcceptor::TcpAcceptorPtr m_acceptor;

    NetAddr::NetAddrPtr m_local_addr; // 本地监听地址

    EventLoop* m_main_eventloop {NULL}; // mainReactor

    IOThreadGroup* m_io_thread_groups {NULL}; // subReactor

    FdEvent* m_listen_fd_event;

    int m_client_counts {0};

    std::set<TcpConnection::TcpConnectionPtr> m_client;

    TimerEvent::TimerEventPtr m_clear_client_timer_event;

};
}

#endif