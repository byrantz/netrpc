#ifndef NETRPC_NET_RPC_RPC_CLOSURE_H
#define NETRPC_NET_RPC_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>

namespace netrpc {

class RpcClosure : public google::protobuf::Closure {
public:
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