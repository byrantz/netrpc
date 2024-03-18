#ifndef NETRPC_NET_RPC_RPC_CONTROLLER_H
#define NETRPC_NET_RPC_RPC_CONTROLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <string>

#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/common/log.h"

namespace netrpc {

class RpcController : public google::protobuf::RpcController {
public:
    RpcController() {INFOLOG("RpcController");}
    ~RpcController() {INFOLOG("~RpcController");}

    void Reset();

    bool Failed() const;

    std::string ErrorText() const;

    void StartCancel();

    void SetFailed(const std::string& reason);

    bool IsCanceled() const;

    void NotifyOnCancel(google::protobuf::Closure* callback);

    void SetError(int32_t error_code, const std::string error_info);

    int32_t GetErrorCode();

    std::string GetErrorInfo();

    void SetMsgId(const std::string& msg_id);

    std::string GetMsgId();

    void SetLocalAddr(NetAddr::NetAddrPtr addr);

    void SetPeerAddr(NetAddr::NetAddrPtr addr);

    NetAddr::NetAddrPtr GetLocalAddr();

    NetAddr::NetAddrPtr GetPeerAddr();

    void SetTimeout(int timeout);

    int GetTimeout();

    bool Finished();

    void SetFinished(bool value);

    int GetMaxRetry();

    void SetMaxRetry(const int maxRetry);

private:
    int32_t m_error_code {0};
    std::string m_error_info;
    std::string m_msg_id;

    bool m_is_failed {false};
    bool m_is_canceled {false};
    bool m_is_finished {false};

    NetAddr::NetAddrPtr m_local_addr;
    NetAddr::NetAddrPtr m_peer_addr;

    int m_timeout {1000}; // ms
    int m_max_retry {2}; // 客户端最大重试次数
};

}

#endif