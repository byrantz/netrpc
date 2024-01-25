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

class OrderImpl : public Order {

public:
    void makeOrder(google::protobuf::RpcController* controller,
                        const ::makeOrderRequest* request,
                        ::makeOrderResponse* response,
                        ::google::protobuf::Closure* done) {

        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_order_id("20240125");
    }
};

void test_tcp_server() {
    netrpc::IPNetAddr::NetAddrPtr addr = std::make_shared<netrpc::IPNetAddr>("127.0.0.1", 12345);

    DEBUGLOG("create addr %s", addr->toString().c_str());

    netrpc::TcpServer tcp_server(addr);

    tcp_server.start();
}

int main() {

    netrpc::Config::GetInst().Init("../conf/netrpc.xml");
    netrpc::Logger::GetInst().Init();

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    netrpc::RpcDispatcher::GetInst().registerService(service);

    test_tcp_server();

    return 0;
}