#ifndef NETRPC_COMMON_EXCEPTION_H
#define NETRPC_COMMON_EXCEPTION_H

#include <exception>

namespace netrpc {

class NetrpcException : public std::exception {
public :
    NetrpcException() = default;

    // 异常处理
    // 当 EventLoop 捕获到 NetrpcException 及其子类对象的异常时，会执行该函数
    virtual void handle() = 0;

    virtual ~NetrpcException() {};

};

}

#endif