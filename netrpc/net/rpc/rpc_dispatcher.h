#ifndef NETRPC_NET_RPC_RPC_DISPATCHER_H
#define NETRPC_NET_RPC_RPC_DISPATCHER_H

#include <map>
#include <memory>
#include <google/protobuf/service.h>

#include "netrpc/net/coder/abstract_protocol.h"
#include "netrpc/net/coder/tinypb_protocol.h"

namespace netrpc {

class TcpConnection;

class RpcDispatcher {

public:
    // Singleton, delete copy construct
    RpcDispatcher(const RpcDispatcher&) = delete;
    RpcDispatcher& operator=(const RpcDispatcher&) = delete;
    static RpcDispatcher& GetInst() {
        static RpcDispatcher inst;
        return inst;
    }

public:
    using ServicePtr = std::shared_ptr<google::protobuf::Service>;

    void dispatch(AbstractProtocol::AbstractProtocolPtr request, AbstractProtocol::AbstractProtocolPtr response, TcpConnection* connection);

    void registerService(ServicePtr service);

    void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info);

private:
    bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);

private:
    RpcDispatcher() = default;
    struct ServiceInfo
    {
        ServicePtr m_service; // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 保存服务方法
    };   
    std::map<std::string, ServiceInfo> m_service_map;
};

}

#endif