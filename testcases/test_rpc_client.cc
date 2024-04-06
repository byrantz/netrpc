#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include <google/protobuf/service.h>
#include "netrpc/common/log.h"
#include "netrpc/common/config.h"
#include "netrpc/common/log.h"
#include "netrpc/net/tcp/tcp_client.h"
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/coder/string_coder.h"
#include "netrpc/net/coder/abstract_protocol.h"
#include "netrpc/net/coder/tinypb_coder.h"
#include "netrpc/net/coder/tinypb_protocol.h"
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_server.h"
#include "netrpc/net/rpc/rpc_dispatcher.h"
#include "netrpc/net/rpc/rpc_controller.h"
#include "netrpc/net/rpc/rpc_channel.h"
#include "netrpc/net/rpc/rpc_closure.h"

#include "order.pb.h"

// void test_tcp_client() {
//     netrpc::IPNetAddr::NetAddrPtr addr = std::make_shared<netrpc::IPNetAddr>("127.0.0.1", 12346);
//     netrpc::TcpClient client(addr);
//     client.connect([addr, &client]() {
//         DEBUGLOG("connect to [%s] success", addr->toString().c_str());
//         std::shared_ptr<netrpc::TinyPBProtocol> message = std::make_shared<netrpc::TinyPBProtocol>();
//         message->m_msg_id = "99998888";
//         message->m_pb_data = "test pb data";

//         makeOrderRequest request;
//         request.set_price(100);
//         request.set_goods("apple");

//         if (!request.SerializeToString(&(message->m_pb_data))) {
//             ERRORLOG("serilize error");
//             return;
//         }

//         message->m_method_name = "Order.makeOrder";

//         client.writeMessage(message, [request](netrpc::AbstractProtocol::AbstractProtocolPtr msg_ptr) {
//             DEBUGLOG("send message success, request[%s]", request.ShortDebugString().c_str());
//         });

//         client.readMessage("99998888", [](netrpc::AbstractProtocol::AbstractProtocolPtr msg_ptr) {
//             std::shared_ptr<netrpc::TinyPBProtocol> message = std::dynamic_pointer_cast<netrpc::TinyPBProtocol>(msg_ptr);
//             DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id.c_str(), message->m_pb_data.c_str());
//             makeOrderResponse response;

//             if (!response.ParseFromString(message->m_pb_data)) {
//                 ERRORLOG("deserialize error");
//                 return;
//             }
//             DEBUGLOG("get response success, response[%s]", response.ShortDebugString().c_str());
//         });

//     });
// }

void test_rpc_channel() {
    std::string service_name = "Order"; 
    std::string method_name = "makeOrder"; 
    netrpc::ZkClient zkCli; 
    zkCli.Start(); 
    std::string method_path = "/" + service_name + "/" + method_name; 
    std::string host_data = zkCli.GetData(method_path.c_str()); 
    NEWRPCCHANNEL(host_data, channel);

    NEWMESSAGE(makeOrderRequest, request);
    NEWMESSAGE(makeOrderResponse, response);

    request->set_price(100);
    request->set_goods("apple");

    NEWRPCCONTROLLER(controller);
    controller->SetMsgId("99998888");

    controller->SetTimeout(10000);

    std::shared_ptr<netrpc::RpcClosure> closure = std::make_shared<netrpc::RpcClosure>(nullptr,[request, response, channel, controller]() mutable {
        if (controller->GetErrorCode() == 0) {
            INFOLOG("call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
            // 执行业务逻辑
            if (response->order_id() == "xxx") {
                // xx
            }  
        } else {
            ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]", request->ShortDebugString().c_str(), controller->GetErrorCode(), controller->GetErrorInfo().c_str());
        }
        INFOLOG("now exit eventloop");
        // channel->getTcpClient()->stop(); // 执行这句话，不能看日志效果
        // channel.reset();
    });

    // channel->Init(controller, request, response, closure);

    // Order_Stub stub(channel.get());

    // stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());

    CALLRPC(host_data, Order_Stub, makeOrder, controller, request, response, closure);
}

int main() {
    netrpc::Logger::GetInst().Init(0);
    netrpc::Config::GetInst().Init("../conf/netrpc.xml");

    // test_connect();
    // test_tcp_client();
    test_rpc_channel();

    INFOLOG("test_rpc_channel end");
    return 0;
}