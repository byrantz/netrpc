# NetRPC框架
## 项目介绍
NetRPC是一个为Linux设计的轻量级、高性能C++ RPC框架。它专注于提供高效的RPC方法调用，实现服务器之间的快速可靠通信。

## 环境配置
### 前置条件
* Linux操作系统（推荐Ubuntu 20.04）
* 支持C++11的GCC/G++编译器
* 使用VsCode通过SSH进行远程开发

### 依赖安装
* Protobuf 3.19.4或更高版本
* TinyXML用于配置管理

### 安装指南
Protobuf
```
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz
tar -xzvf protobuf-cpp-3.19.4.tar.gz
cd protobuf-cpp-3.19.4
./configure --prefix=/usr/local
make -j4
sudo make install
```

TinyXML
```
wget https://udomain.dl.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip
unzip tinyxml_2_6_2.zip
```


## 运行项目
在根目录下直接执行 `make` 命令，之后到 `bin` 目录下启动RPC服务器和客户端进行测试，请使用以下命令：

**启动RPC服务器 **

```
./test_rpc_server 
```

**启动RPC客户端**

```
./test_rpc_client
```
