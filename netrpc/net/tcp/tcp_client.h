#ifndef NETRPC_NET_TCP_TCP_CLIENT_H
#define NETRPC_NET_TCP_TCP_CLIENT_H

#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/net/tcp/tcp_connection.h"
#include "netrpc/net/coder/abstract_protocol.h"

namespace netrpc {

class TcpClient {
public:
    TcpClient(NetAddr::NetAddrPtr peer_addr);

    ~TcpClient();

    // 异步的进行 connect
    // 如果 connect 成功，done 会被执行
    void connect(std::function<void()> done);

    // 异步的发送 message
    // 如果发送 message 成功，会调用 done 函数，函数的入参就是 message 对象
    void writeMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

    // 异步的读取 message
    // 如果读取 message 成功，会调用 done 函数，函数的入参就是 message 对象
    void readMessage(const std::string& req_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

private:
    NetAddr::NetAddrPtr m_peer_addr;
    EventLoop* m_eventloop {NULL};

    int m_fd {-1};
    FdEvent* m_fd_event {NULL};

    TcpConnection::TcpConnectionPtr m_connection;
};

}
#endif