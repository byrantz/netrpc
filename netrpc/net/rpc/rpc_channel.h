#ifndef NETRPC_NET_RPC_RPC_CHANNEL_H
#define NETRPC_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include <memory>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_client.h"
#include "netrpc/net/timer_event.h"

namespace netrpc {

#define NEWMESSAGE(type, var_name) \
    std::shared_ptr<type> var_name = std::make_shared<type>(); \

#define NEWRPCCONTROLLER(var_name) \
    std::shared_ptr<netrpc::RpcController> var_name = std::make_shared<netrpc::RpcController>(); \

#define NEWRPCCHANNEL(addr, var_name) \
    std::shared_ptr<netrpc::RpcChannel> var_name = std::make_shared<netrpc::RpcChannel>(std::make_shared<netrpc::IPNetAddr>(addr)); \

#define CALLRPC(addr, method_name, controller, request, response, closure) \
    { \
    NEWRPCCHANNEL(addr, channel); \
    channel->Init(controller, request, response, closure); \
    Order_Stub(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
    } \

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

    TimerEvent::TimerEventPtr getTimerEvent();

private:
    NetAddr::NetAddrPtr m_peer_addr {nullptr};
    NetAddr::NetAddrPtr m_local_addr {nullptr};

    ControllerPtr m_controller {nullptr};
    MessagePtr m_request {nullptr};
    MessagePtr m_response {nullptr};
    ClosurePtr m_closure {nullptr};

    bool m_is_init {false};

    TcpClient::TcpClientPtr m_client {nullptr};

    TimerEvent::TimerEventPtr m_timer_event{nullptr};
};

}

#endif