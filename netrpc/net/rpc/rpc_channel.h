#ifndef NETRPC_NET_RPC_RPC_CHANNEL_H
#define NETRPC_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include <memory>
#include <vector>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/common/zookeeperutil.h"
#include "netrpc/net/tcp/tcp_client.h"
#include "netrpc/net/timer_event.h"
#include "netrpc/net/load_balance.h"

namespace netrpc {

#define NEWMESSAGE(type, var_name) \
    std::shared_ptr<type> var_name = std::make_shared<type>(); \

#define NEWRPCCONTROLLER(var_name) \
    std::shared_ptr<netrpc::RpcController> var_name = std::make_shared<netrpc::RpcController>(); \

#define NEWRPCCHANNEL(addr, var_name) \
    std::shared_ptr<netrpc::RpcChannel> var_name = std::make_shared<netrpc::RpcChannel>(netrpc::RpcChannel::FindAddr(addr)); \

// stub_name 这一步里面会调用 RpcChannel::CallMethod
#define CALLRPC(addr, stub_name, method_name, controller, request, response, closure) \
    { \
    NEWRPCCHANNEL(addr, channel); \
    channel->Init(controller, request, response, closure); \
    stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
    } \

class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel> {

public:
    using RpcChannelPtr = std::shared_ptr<RpcChannel>;
    using ControllerPtr = std::shared_ptr<google::protobuf::RpcController>;
    using MessagePtr = std::shared_ptr<google::protobuf::Message>;
    using ClosurePtr = std::shared_ptr<google::protobuf::Closure>; 

public:
    // 获取 addr
    // 若 str 是 ip:port，直接返回
    // 否则认为是 rpc 服务名，尝试从配置文件里面获取对应的 ip:port
    static NetAddr::NetAddrPtr FindAddr(const std::string& str);

public:
    RpcChannel(NetAddr::NetAddrPtr peer_addr);

    RpcChannel(std::vector<NetAddr::NetAddrPtr> addrs, LoadBalanceCategory loadBalance = LoadBalanceCategory::Random);

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
    void callBack();

private:
    NetAddr::NetAddrPtr m_peer_addr {nullptr};
    NetAddr::NetAddrPtr m_local_addr {nullptr};

    ControllerPtr m_controller {nullptr};
    MessagePtr m_request {nullptr};
    MessagePtr m_response {nullptr};
    ClosurePtr m_closure {nullptr};

    bool m_is_init {false};

    TcpClient::TcpClientPtr m_client {nullptr};

    std::vector<NetAddr::NetAddrPtr> m_addrs;
    LoadBalanceStrategy::ptr m_loadBalancer;
};

}

#endif