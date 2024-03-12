#ifndef NETRPC_NET_RPC_RPC_CLOSURE_H
#define NETRPC_NET_RPC_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
#include "netrpc/common/run_time.h"
#include "netrpc/common/log.h"
#include "netrpc/common/exception.h"
#include "netrpc/net/rpc/rpc_interface.h"

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
            if (m_rpc_interface) {
                m_rpc_interface.reset();
            }
        } catch (NetrpcException& e) {
            ERRORLOG("RocketException exception[%s], deal handle", e.what());
            e.handle();
            if (m_rpc_interface) {
                m_rpc_interface->setError(e.errorCode(), e.errorInfo());
                m_rpc_interface.reset();
            }
        } catch (std::exception& e) {
            ERRORLOG("std::exception[%s]", e.what());
            if (m_rpc_interface) {
                m_rpc_interface->setError(-1, "unknown std::exception");
                m_rpc_interface.reset();
            }
        }
    }
private:
    std::function<void()> m_cb {nullptr};
    RpcInterfacePtr m_rpc_interface{nullptr};
};

}

#endif