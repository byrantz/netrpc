#ifndef NETRPC_NET_RPC_RPC_CLOSURE_H
#define NETRPC_NET_RPC_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
#include "netrpc/common/run_time.h"
#include "netrpc/common/log.h"
#include "netrpc/common/exception.h"

namespace netrpc {


class RpcClosure : public google::protobuf::Closure {
public:
    RpcClosure(std::function<void()> cb) : m_cb(cb) {
        INFOLOG("RpcClosure");
    }

    ~RpcClosure() {
        INFOLOG("~RpcClosure");
    }
    
    void Run() override {

        if (m_cb != nullptr) {
            m_cb();
        }
    }
private:
    std::function<void()> m_cb {nullptr};
};

}

#endif