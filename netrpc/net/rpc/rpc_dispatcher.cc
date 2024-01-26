#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "netrpc/net/rpc/rpc_dispatcher.h"
#include "netrpc/net/coder/tinypb_protocol.h"
#include "netrpc/common/log.h"
#include "netrpc/common/error_code.h"
#include "netrpc/net/rpc/rpc_controller.h"
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_connection.h"

namespace netrpc {

void RpcDispatcher::dispatch(AbstractProtocol::AbstractProtocolPtr request, AbstractProtocol::AbstractProtocolPtr response, TcpConnection* connection) {
    std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);

    std::string method_full_name = req_protocol->m_method_name;
    std::string service_name;
    std::string method_name;

    rsp_protocol->m_msg_id = req_protocol->m_msg_id;
    rsp_protocol->m_method_name = req_protocol->m_method_name;

    if (!parseServiceFullName(method_full_name, service_name, method_name)) {
        setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
        return;
    } 

    auto it = m_service_map.find(service_name);
    if (it == m_service_map.end()) {
        ERRORLOG("%s | service name[%s] not found", req_protocol->m_msg_id.c_str(), service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
        return;
    }

    ServicePtr service = (*it).second;

    const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
    if (method == NULL) {
        ERRORLOG("%s | method name[%s] not found in service[%s]", req_protocol->m_msg_id.c_str(), method_name.c_str(), service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "method not found");
        return;
    }

    google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();

    // 反序列化，将 pb_data 反序列化为 req_msg
    if (!req_msg->ParseFromString(req_protocol->m_pb_data)) {
        ERRORLOG("%s | deserilize error", req_protocol->m_msg_id.c_str(), method_name.c_str(), service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
        if (req_msg != NULL) {
            delete req_msg;
            req_msg = NULL;
        }
        return;
    }

    INFOLOG("%s | get rpc request[%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str());

    google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();

    RpcController rpcController;
    rpcController.SetLocalAddr(connection->getLocalAddr());
    rpcController.SetPeerAddr(connection->getPeerAddr());
    rpcController.SetMsgId(req_protocol->m_msg_id);

    service->CallMethod(method, &rpcController, req_msg, rsp_msg, NULL);
    if (!rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data))) {
        ERRORLOG("%s | deserilize error", req_protocol->m_msg_id.c_str(), method_name.c_str(), service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "serilize error");
        if (req_msg != NULL) {
            delete req_msg;
            req_msg = NULL;
        }
        if (rsp_msg != NULL) {
            delete rsp_msg;
            rsp_msg = NULL;
        }

        return;
    }

    rsp_protocol->m_err_code = 0;
    INFOLOG("%s | dispatch success, request[%s], response[%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str(), rsp_msg->ShortDebugString().c_str());

    delete req_msg;
    delete rsp_msg;
    req_msg = NULL;
    rsp_msg = NULL;
}

void RpcDispatcher::registerService(ServicePtr service) {
    std::string service_name = service->GetDescriptor()->full_name();
    m_service_map[service_name] = service;
}

void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info) {
    msg->m_err_code = err_code;
    msg->m_err_info = err_info;
    msg->m_err_info_len = err_info.length();
}

bool RpcDispatcher::parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name) {
    if (full_name.empty()) {
        ERRORLOG("full name empty()");
        return false;
    }
    size_t i = full_name.find_first_of(".");
    if (i == full_name.npos) {
        ERRORLOG("not find . in full name [%s]", full_name.c_str());
        return false;
    }

    service_name = full_name.substr(0, i);
    method_name = full_name.substr(i + 1, full_name.length() - i - 1);

    INFOLOG("parse service_name[%s] and method_name[%s] from full name [%s]", service_name.c_str(), method_name.c_str(), full_name.c_str());

    return true;
}

}