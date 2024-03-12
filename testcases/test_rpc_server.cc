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
        APPDEBUGLOG("start sleep 5s");
        sleep(5);
        APPDEBUGLOG("end sleep 5s");
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_order_id("20240125");
        APPDEBUGLOG("call makeOrder success");
        if (done) {
            done->Run();
            delete done;
            done = NULL;
        }
    }
};


int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     printf("Start test_rpc_server error, argc not 2 \n");
    //     printf("Start like this: \n");
    //     printf("./test_rpc_server ../conf/netrpc.xml \n");
    //     return 0;
    // }

    netrpc::Config::GetInst().Init("../conf/netrpc.xml");
    netrpc::Logger::GetInst().Init(1);

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    netrpc::RpcDispatcher::GetInst().registerService(service);

    netrpc::IPNetAddr::NetAddrPtr addr = std::make_shared<netrpc::IPNetAddr>("127.0.0.1", netrpc::Config::GetInst().m_port);

    netrpc::TcpServer tcp_server(addr);

    tcp_server.start();

    return 0;
}