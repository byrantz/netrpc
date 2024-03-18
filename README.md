# NetRPC框架
## 项目介绍
NetRPC 是一个为 Linux 设计的轻量级、高性能 C++ RPC 框架。负责解决在分布式服务部署中服务的发布与调用、消息的序列和反序列化、网络包的收发等问题，使其能提供高并发的远程函数调用服务，可以让使用者专注于业务，快速实现微服务的分布式部署。

## Features
+ 并发模型采用 Epoll LT + 主从 Readtor + 非阻塞 I/O + 线程池，使用轮询方式将新连接分给其中一个 Reactor 线程。
+ 基于 Protobuf 实现了自定义的通信协议，来传输序列化后的 RPC 调用数据
+ 实现了异步日志系统，并分开存放框架日志和业务日志，在不影响服务器运行效率的同时记录服务器的运行状态，便于排查问题。
+ 利用 timerfd 提供毫秒级精度的定时器，为异步日志提供支持
+ 在 RPC 客户端实现了简单的 RPC 调用异常重试的机制，提升了框架的容错性





## 环境配置
### 前置条件
* Linux操作系统（推荐Ubuntu 20.04）
* 支持C++11的GCC/G++编译器

### 依赖安装
* Protobuf 3.19.4或更高版本
* TinyXML用于配置管理

### 安装指南
**安装 Protobuf**
```
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz
tar -xzvf protobuf-cpp-3.19.4.tar.gz
cd protobuf-cpp-3.19.4
./configure --prefix=/usr/local
make -j4
sudo make install
```

**安装 TinyXML**
```
wget https://udomain.dl.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip
unzip tinyxml_2_6_2.zip
```


## 运行项目

编写RPC方法使用，这里以 `order.proto` 编写服务， `test_rpc_server.cc` 和 `test_rpc_client.cc` 编写服务发布端和服务调用端为例。

```proto
syntax = "proto3";
option cc_generic_services = true;

// 定义请求消息
message makeOrderRequest {
    int32 price = 1;
    string goods = 2;
}

// 定义响应消息
message makeOrderResponse {
    int32 ret_code = 1;
    string res_info = 2;
    string order_id = 3;
}

// 定义 RPC 服务
service Order {
    rpc makeOrder(makeOrderRequest) returns (makeOrderResponse);
}

```
到 `testcases` 目录下, 执行 protoc 编译 
```shell
cd testcase
protoc -I=. order.proto -cpp_out=.
```
&nbsp;

**服务发布端**
```cpp
#include "order.pb.h"

// 实现服务
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
    netrpc::Config::GetInst().Init("../conf/netrpc.xml");
    netrpc::Logger::GetInst().Init(1);

    // 注册 OrderImpl 服务
    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    netrpc::RpcDispatcher::GetInst().registerService(service);

    // 设置发布的 ip:port
    netrpc::IPNetAddr::NetAddrPtr addr = std::make_shared<netrpc::IPNetAddr>("127.0.0.1", netrpc::Config::GetInst().m_port);

    netrpc::TcpServer tcp_server(addr);

    // 启动服务等待调用
    tcp_server.start();

    return 0;
}
```
&nbsp;

**服务调用端**
```cpp
void test_rpc_channel() {
    NEWRPCCHANNEL("127.0.0.1:12345", channel);

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
        channel->getTcpClient()->stop(); 
    });

    CALLRPC("127.0.0.1:12345", Order_Stub, makeOrder, controller, request, response, closure);
}

int main() {
    netrpc::Logger::GetInst().Init(0);

    // test_connect();
    // test_tcp_client();
    test_rpc_channel();

    INFOLOG("test_rpc_channel end");
    return 0;
}
```
&nbsp;


编写完成后，在根目录下直接执行 `sudo sh build.sh` 命令，之后到 `bin` 目录下启动RPC服务器和客户端进行测试，请使用以下命令：

**启动 RPC 服务器**

```
./test_rpc_server 
```

**启动 RPC 客户端**

```
./test_rpc_client
```


## TODO

- 实现基于 HTTP 协议的通信，进行压力测试
- 使用 Zookeeper 实现服务发现功能


