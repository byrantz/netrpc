# NetRPC框架
## 项目介绍
NetRPC 是一个为 Linux 设计的轻量级、高性能 C++ RPC 框架。负责解决在分布式服务部署中服务的发布与调用、消息的序列和反序列化、网络包的收发等问题，使其能提供高并发的远程函数调用服务，可以让使用者专注于业务，快速实现微服务的分布式部署。

## 主要功能
**服务端发布** ：支持将本地服务注册到 rpc 节点上并启动该服务，等待远程的 rpc 调用；支持执行回调操作，完成响应对象数据的序列化和网络发送

**服务调用端** ：支持调用服务发布端的某已注册的服务，并接受发布端返回的响应数据

**日志功能** ：支持分级设置日志级别，方便调试



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
#include "order.pb.h"

void test_rpc_channel() {
    // 服务发布端的地址为 127.0.0.1:12345
    NEWRPCCHANNEL("127.0.0.1:12345", channel);

    NEWMESSAGE(makeOrderRequest, request);
    NEWMESSAGE(makeOrderResponse, response);

    // 设置请求参数
    request->set_price(100);
    request->set_goods("apple");

    NEWRPCCONTROLLER(controller);
    // 设置唯一的消息 id
    controller->SetMsgId("99998888");
    // 设置超时时间
    controller->SetTimeout(10000);
    // 设置 Rpc 成功后的回调函数
    std::shared_ptr<netrpc::RpcClosure> closure = std::make_shared<netrpc::RpcClosure>([request, response, channel, controller]() mutable {
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
        channel.reset();
    });

    // 调用 RPC 服务
    CALLRPC("127.0.0.1:12345", Order_Stub, makeOrder, controller, request, response, closure);
}

int main() {
    // netrpc::Config::GetInst().Init("../conf/netrpc_client.xml");
    netrpc::Logger::GetInst().Init(0);

    test_rpc_channel();
    return 0;
}
```
&nbsp;


编写完成后，在根目录下直接执行 `make` 命令，之后到 `bin` 目录下启动RPC服务器和客户端进行测试，请使用以下命令：

**启动 RPC 服务器**

```
./test_rpc_server 
```

**启动 RPC 客户端**

```
./test_rpc_client
```


