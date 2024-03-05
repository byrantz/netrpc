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
#include "netrpc/common/log.h"
#include "netrpc/common/config.h"
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_client.h"
#include "netrpc/net/coder/string_coder.h"
#include "netrpc/net/coder/abstract_protocol.h"
#include "netrpc/net/coder/tinypb_coder.h"
#include "netrpc/net/coder/tinypb_protocol.h"

void test_connect() {
    // 调用 connect 连接 server
    // write 一个字符串
    // 等待 read 返回结果

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

    DEBUGLOG("connect success");

    std::string msg = "hello netrpc!";

    rt = write(fd, msg.c_str(), msg.length());

    DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

    char buf[100];
    rt = read(fd, buf, 100);
    DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());
}

void test_tcp_client() {
    netrpc::IPNetAddr::NetAddrPtr addr = std::make_shared<netrpc::IPNetAddr>("127.0.0.1", 12345);
    netrpc::TcpClient client(addr);
    client.connect([addr, &client]() {
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        std::shared_ptr<netrpc::TinyPBProtocol> message = std::make_shared<netrpc::TinyPBProtocol>();
        message->m_msg_id = "123456789";
        message->m_pb_data = "test pb data";
        client.writeMessage(message, [](netrpc::AbstractProtocol::AbstractProtocolPtr msg_ptr) {
            DEBUGLOG("send message success");
        });

        client.readMessage("123456789", [](netrpc::AbstractProtocol::AbstractProtocolPtr msg_ptr) {
            std::shared_ptr<netrpc::TinyPBProtocol> message = std::dynamic_pointer_cast<netrpc::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id.c_str(), message->m_pb_data.c_str());
        });

        client.writeMessage(message, [](netrpc::AbstractProtocol::AbstractProtocolPtr msg_ptr) {
            DEBUGLOG("send message 22222 success");
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