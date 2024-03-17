#ifndef NETRPC_NET_TCP_TCP_ACCEPTOR_H
#define NETRPC_NET_TCP_TCP_ACCEPTOR_H

#include "netrpc/net/tcp/net_addr.h"
#include <memory>

namespace netrpc {

class TcpAcceptor {
public:
    using TcpAcceptorPtr = std::shared_ptr<TcpAcceptor>;

    TcpAcceptor(NetAddr::NetAddrPtr local_addr);

    ~TcpAcceptor();

    std::pair<int, NetAddr::NetAddrPtr> accept();

    int getListenFd();

private:
    NetAddr::NetAddrPtr m_local_addr; // 服务端监听的地址，addr -> ip:port

    // 协议簇
    int m_family {-1};

    int m_listenfd {-1}; // 监听套接字
};
}

#endif