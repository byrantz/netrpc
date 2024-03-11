#ifndef NETRPC_NET_RPC_RPC_CLOSURE_H
#define NETRPC_NET_RPC_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
#include "netrpc/common/run_time.h"
#include "netrpc/common/log.h"
#include "netrpc/common/exception.h"

namespace netrpc {

class RpcInterface;

class RpcClosure : public google::protobuf::Closure {
public:
    using RpcInterfacePtr = std::shared_ptr<RpcInterface>;

    RpcClosure(RpcInterfacePtr interface, std::function<void()> cb) : m_rpc_interface(interface), m_cb(cb) {
        INFOLOG("RpcClosure");
    }

    ~RpcClosure() {
        INFOLOG("~RpcClosure");
    }
    
    void Run() override {
        if (!m_rpc_interface) {
        RunTime::GetRunTime()->m_rpc_interface = m_rpc_interface.get();
        }
        try {
        if (m_cb != nullptr) {
            m_cb();
        }
        } catch (NetrpcException& e) {
            ERRORLOG("RocketException exception[%s], deal handle", e.what());
            e.handle();
        } catch (std::exception& e) {
            ERRORLOG("std::exception[%s]", e.what());
        }
    }
private:
    std::function<void()> m_cb {nullptr};
    RpcInterfacePtr m_rpc_interface{nullptr};
};

}

#endif