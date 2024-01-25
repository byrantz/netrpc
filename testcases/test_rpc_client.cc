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
#include "order.pb.h"

void test_tcp_client() {
    netrpc::IPNetAddr::NetAddrPtr addr = std::make_shared<netrpc::IPNetAddr>("127.0.0.1", 12345);
    netrpc::TcpClient client(addr);
    client.connect([addr, &client]() {
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        std::shared_ptr<netrpc::TinyPBProtocol> message = std::make_shared<netrpc::TinyPBProtocol>();
        message->m_req_id = "99998888";
        message->m_pb_data = "test pb data";

        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");

        if (!request.SerializeToString(&(message->m_pb_data))) {
            ERRORLOG("serilize error");
            return;
        }

        message->m_method_name = "Order.makeOrder";

        client.writeMessage(message, [request](netrpc::AbstractProtocol::AbstractProtocolPtr msg_ptr) {
            DEBUGLOG("send message success, request[%s]", request.ShortDebugString().c_str());
        });

        client.readMessage("99998888", [](netrpc::AbstractProtocol::AbstractProtocolPtr msg_ptr) {
            std::shared_ptr<netrpc::TinyPBProtocol> message = std::dynamic_pointer_cast<netrpc::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("req_id[%s], get response %s", message->m_req_id.c_str(), message->m_pb_data.c_str());
            makeOrderResponse response;

            if (!response.ParseFromString(message->m_pb_data)) {
                ERRORLOG("deserialize error");
                return;
            }
            DEBUGLOG("get response success, response[%s]", response.ShortDebugString().c_str());
        });

    });
}

int main() {
    netrpc::Config::GetInst().Init("../conf/netrpc.xml");
    
    netrpc::Logger::GetInst().Init();

    // test_connect();
    test_tcp_client();
    return 0;
}