#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "netrpc/net/rpc/rpc_channel.h"
#include "netrpc/net/rpc/rpc_controller.h"
#include "netrpc/net/coder/tinypb_protocol.h"
#include "netrpc/net/tcp/tcp_client.h"
#include "netrpc/common/log.h"
#include "netrpc/common/msg_id_util.h"
#include "netrpc/common/error_code.h"
#include "netrpc/net/timer_event.h"

namespace netrpc {

RpcChannel::RpcChannel(NetAddr::NetAddrPtr peer_addr): m_peer_addr(peer_addr) {
  INFOLOG("RpcChannel");
  m_client = std::make_shared<TcpClient>(m_peer_addr);
}

RpcChannel::~RpcChannel() {
    INFOLOG("~RpcChannel");
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                        google::protobuf::Message* response, google::protobuf::Closure* done) {

    std::shared_ptr<netrpc::TinyPBProtocol> req_protocol = std::make_shared<netrpc::TinyPBProtocol>();

    RpcController* my_controller = dynamic_cast<RpcController*>(controller);
    if (my_controller == NULL) {
        ERRORLOG("failed callmethod, RpcController convert error");
        return;
    }

    if (my_controller->GetMsgId().empty()) {
        req_protocol->m_msg_id = MsgIDUtil::GenMsgID();
        my_controller->SetMsgId(req_protocol->m_msg_id);
    } else {
        req_protocol->m_msg_id = my_controller->GetMsgId();
    }

    req_protocol->m_method_name = method->full_name();
    INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

    if (!m_is_init) {
        std::string err_info = "RpcChannel not init";
        my_controller->SetError(ERROR_RPC_CHANNEL_INIT, err_info);
        ERRORLOG("%s | %s, RpcChannel not init", req_protocol->m_msg_id.c_str(), err_info.c_str());
        return;
    }

    // request 的序列化
    if (!request->SerializePartialToString(&(req_protocol->m_pb_data))) {
        std::string err_info = "failed to serialize";
        my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
        ERRORLOG("%s | %s, origin request [%s]", req_protocol->m_msg_id.c_str(), err_info.c_str(), request->ShortDebugString().c_str());
        return;
    }

    RpcChannelPtr channel = shared_from_this();

    m_timer_event = std::make_shared<TimerEvent>(my_controller->GetTimeout(), false, [my_controller, channel]() mutable {
        my_controller->StartCancel();
        my_controller->SetError(ERROR_RPC_CALL_TIMEOUT, "rpc call timeout " + std::to_string(my_controller->GetTimeout()));

        if (channel->getClosure()) {
            channel->getClosure()->Run();
        }
        channel.reset();
    });

    m_client->addTimerEvent(m_timer_event);

    m_client->connect([req_protocol, channel]() mutable {

    RpcController* my_controller = dynamic_cast<RpcController*>(channel->getController());

    if (channel->getTcpClient()->getConnectErrorCode() != 0) {
      my_controller->SetError(channel->getTcpClient()->getConnectErrorCode(), channel->getTcpClient()->getConnectErrorInfo());
      ERRORLOG("%s | connect error, error coode[%d], error info[%s], peer addr[%s]", 
        req_protocol->m_msg_id.c_str(), my_controller->GetErrorCode(), 
        my_controller->GetErrorInfo().c_str(), channel->getTcpClient()->getPeerAddr()->toString().c_str());

        // 取消定时任务
        channel->getTimerEvent()->setCancled(true);
        if (channel->getClosure()) {
          channel->getClosure()->Run();
        }
        channel.reset();
      return;
    }

    INFOLOG("%s | connect success, peer addr[%s], local addr[%s]",
      req_protocol->m_msg_id.c_str(), 
      channel->getTcpClient()->getPeerAddr()->toString().c_str(), 
      channel->getTcpClient()->getLocalAddr()->toString().c_str()); 

    channel->getTcpClient()->writeMessage(req_protocol, [req_protocol, channel, my_controller](AbstractProtocol::AbstractProtocolPtr) mutable {
      INFOLOG("%s | send rpc request success. call method name[%s], peer addr[%s], local addr[%s]", 
        req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str(),
        channel->getTcpClient()->getPeerAddr()->toString().c_str(), channel->getTcpClient()->getLocalAddr()->toString().c_str());

      channel->getTcpClient()->readMessage(req_protocol->m_msg_id, [channel, my_controller](AbstractProtocol::AbstractProtocolPtr msg) mutable {
        std::shared_ptr<netrpc::TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<netrpc::TinyPBProtocol>(msg);
        INFOLOG("%s | success get rpc response, call method name[%s], peer addr[%s], local addr[%s]", 
          rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
          channel->getTcpClient()->getPeerAddr()->toString().c_str(), channel->getTcpClient()->getLocalAddr()->toString().c_str());
        
        // 当成功读取到回包后，取消定时任务
        channel->getTimerEvent()->setCancled(true); 

        if (!(channel->getResponse()->ParseFromString(rsp_protocol->m_pb_data))){
          ERRORLOG("%s | serialize error", rsp_protocol->m_msg_id.c_str());
          my_controller->SetError(ERROR_FAILED_SERIALIZE, "serialize error");
          return;
        }

        if (rsp_protocol->m_err_code != 0) {
          ERRORLOG("%s | call rpc methood[%s] failed, error code[%d], error info[%s]", 
            rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
            rsp_protocol->m_err_code, rsp_protocol->m_err_info.c_str());

          my_controller->SetError(rsp_protocol->m_err_code, rsp_protocol->m_err_info);
          return;
        }

        INFOLOG("%s | call rpc success, call method name[%s], peer addr[%s], local addr[%s]",
          rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
          channel->getTcpClient()->getPeerAddr()->toString().c_str(), channel->getTcpClient()->getLocalAddr()->toString().c_str())

        if (!my_controller->IsCanceled() && channel->getClosure()) {
          channel->getClosure()->Run();
        }

        channel.reset();
      });

    });

  });

}

void RpcChannel::Init(ControllerPtr controller, MessagePtr req, MessagePtr res, ClosurePtr done) {
    if (m_is_init) {
        return;
    }
    m_controller = controller;
    m_request = req;
    m_response = res;
    m_closure = done;
    m_is_init = true;
}

google::protobuf::RpcController* RpcChannel::getController() {
    return m_controller.get();
}

google::protobuf::Message* RpcChannel::getResponse() {
    return m_response.get();
}

google::protobuf::Message* RpcChannel::getRequest() {
    return m_request.get();
}

google::protobuf::Closure* RpcChannel::getClosure() {
    return m_closure.get();
}

TcpClient* RpcChannel::getTcpClient() {
    return m_client.get();
}

TimerEvent::TimerEventPtr RpcChannel::getTimerEvent() {
    return m_timer_event;
}

}