#ifndef NETRPC_NET_RPC_RPC_CHANNEL_H
#define NETRPC_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include <memory>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_client.h"

namespace netrpc {

class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel> {

public:
    using RpcChannelPtr = std::shared_ptr<RpcChannel>;
    using ControllerPtr = std::shared_ptr<google::protobuf::RpcController>;
    using MessagePtr = std::shared_ptr<google::protobuf::Message>;
    using ClosurePtr = std::shared_ptr<google::protobuf::Closure>; 

public:
    RpcChannel(NetAddr::NetAddrPtr peer_addr);

    ~RpcChannel();

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                            google::protobuf::Message* response, google::protobuf::Closure* done);

    void Init(ControllerPtr controller, MessagePtr req, MessagePtr res, ClosurePtr done);

    google::protobuf::RpcController* getController();

    google::protobuf::Message* getRequest();

    google::protobuf::Message* getResponse();

    google::protobuf::Closure* getClosure();

    TcpClient* getTcpClient();

private:
    NetAddr::NetAddrPtr m_peer_addr {nullptr};
    NetAddr::NetAddrPtr m_local_addr {nullptr};

    ControllerPtr m_controller {nullptr};
    MessagePtr m_request {nullptr};
    MessagePtr m_response {nullptr};
    ClosurePtr m_closure {nullptr};

    bool m_is_init {false};

    TcpClient::TcpClientPtr m_client {nullptr};
};

}

#endif