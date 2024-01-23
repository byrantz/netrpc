#ifndef NETRPC_NET_TCP_CONNECTION_H
#define NETRPC_NET_TCP_CONNECTION_H

#include <memory>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_buffer.h"
#include "netrpc/net/io_thread.h"

namespace netrpc {

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 4,
};

class TcpConnection {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
public:
    TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::NetAddrPtr peer_addr);

    ~TcpConnection();

    void onRead();

    void excute();

    void onWrite();

    void setState(const TcpState state);

    TcpState getState();

    void clear();

    // 服务器主动关闭连接
    void shutdown();
    
private:
    IOThread* m_io_thread {NULL}; // 代表持有该连接的 IO 线程

    NetAddr::NetAddrPtr m_local_addr;
    NetAddr::NetAddrPtr m_peer_addr;

    TcpBuffer::TcpBufferPtr m_in_buffer;
    TcpBuffer::TcpBufferPtr m_out_buffer;

    FdEvent* m_fd_event {NULL};

    TcpState m_state;

    int m_fd {0};
};

}
#endif