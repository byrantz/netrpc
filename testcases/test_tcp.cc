#include "netrpc/common/log.h"
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_server.h"
#include "netrpc/common/config.h"

void test_tcp_server() {
    netrpc::IPNetAddr::NetAddrPtr addr = std::make_shared<netrpc::IPNetAddr>("127.0.0.1", 12346);

    DEBUGLOG("create addr %s", addr->toString().c_str());

    netrpc::TcpServer tcp_server(addr);

    tcp_server.start();
}

int main() {

    netrpc::Config::GetInst().Init("../conf/netrpc.xml");
    netrpc::Logger::GetInst().Init(1);

    test_tcp_server();
}