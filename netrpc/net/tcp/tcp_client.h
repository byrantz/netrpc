#ifndef NETRPC_NET_TCP_TCP_CLIENT_H
#define NETRPC_NET_TCP_TCP_CLIENT_H

#include <memory>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/net/tcp/tcp_connection.h"
#include "netrpc/net/coder/abstract_protocol.h"
#include "netrpc/net/timer_event.h"

namespace netrpc {

class TcpClient {
public:
    using TcpClientPtr = std::shared_ptr<TcpClient>;

    TcpClient(NetAddr::NetAddrPtr peer_addr);

    ~TcpClient();

    // 异步的进行 connect
    // 如果 connect 完成，done 会被执行
    void connect(std::function<void()> done);

    // 异步的发送 message
    // 如果发送 message 成功，会调用 done 函数，函数的入参就是 message 对象
    void writeMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

    // 异步的读取 message
    // 如果读取 message 成功，会调用 done 函数，函数的入参就是 message 对象
    void readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

    void stop();

    int getConnectErrorCode();

    std::string getConnectErrorInfo();

    NetAddr::NetAddrPtr getPeerAddr();

    NetAddr::NetAddrPtr getLocalAddr();

    void initLocalAddr();

    void addTimerEvent(TimerEvent::TimerEventPtr timer_event);

private:
    NetAddr::NetAddrPtr m_peer_addr;
    NetAddr::NetAddrPtr m_local_addr;

    EventLoop* m_eventloop {NULL};

    int m_fd {-1};
    FdEvent* m_fd_event {NULL};

    TcpConnection::TcpConnectionPtr m_connection;

    int m_connect_error_code {0};
    std::string m_connect_error_info;
};

}
#endif