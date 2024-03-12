#ifndef NETRPC_NET_TCP_CONNECTION_H
#define NETRPC_NET_TCP_CONNECTION_H

#include <memory>
#include <map>
#include <queue>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_buffer.h"
#include "netrpc/net/io_thread.h"
#include "netrpc/net/coder/abstract_coder.h"
#include "netrpc/net/rpc/rpc_dispatcher.h"

namespace netrpc {

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 4,
};

enum TcpConnectionType {
    TcpConnectionByServer = 1, // 作为服务端使用，代表跟对端客户端的连接
    TcpConnectionByClient = 2, // 作为客户端使用，代表跟对端服务端的连接
};

class TcpConnection {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
public:
    TcpConnection(EventLoop* eventloop, int fd, int buffer_size, NetAddr::NetAddrPtr peer_addr, NetAddr::NetAddrPtr local_addr, TcpConnectionType type = TcpConnectionByServer);

    ~TcpConnection();

    void onRead();

    void excute();

    void onWrite();

    void setState(const TcpState state);

    TcpState getState();

    void clear();

    int getFd();

    // 服务器主动关闭连接
    void shutdown();

    void setConnectionType(TcpConnectionType type);

    // 启动监听可写事件
    void listenWrite();

    // 启动监听可读事件
    void listenRead();

    void pushSendMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);
    
    void pushReadMessage(const std::string& msg_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

    NetAddr::NetAddrPtr getLocalAddr();

    NetAddr::NetAddrPtr getPeerAddr();

    void reply(std::vector<AbstractProtocol::AbstractProtocolPtr>& replay_messages);

private:
    EventLoop* m_eventloop {NULL}; // 代表持有该连接的 IO 线程

    NetAddr::NetAddrPtr m_local_addr;
    NetAddr::NetAddrPtr m_peer_addr;

    TcpBuffer::TcpBufferPtr m_in_buffer;
    TcpBuffer::TcpBufferPtr m_out_buffer;

    FdEvent* m_fd_event {NULL};

    AbstractCoder* m_coder {NULL};

    TcpState m_state;

    int m_fd {0};

    TcpConnectionType m_connection_type {TcpConnectionByServer};

    std::vector<std::pair<AbstractProtocol::AbstractProtocolPtr, std::function<void(AbstractProtocol::AbstractProtocolPtr)>>> m_write_dones;

    // key 为 msg_id
    std::map<std::string, std::function<void(AbstractProtocol::AbstractProtocolPtr)>> m_read_dones;
};

}
#endif