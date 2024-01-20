#include "netrpc/common/log.h"
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/common/config.h"

int main() {

    netrpc::Config::GetInst().Init("../conf/netrpc.xml");
    netrpc::Logger::GetInst().Init();

    netrpc::IPNetAddr addr("127.0.0.1", 12345);
    DEBUGLOG("create addr %s", addr.toString().c_str());
}