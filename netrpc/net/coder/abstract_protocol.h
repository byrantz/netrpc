#ifndef NETRPC_NET_ABSTRACT_PROTOCOL_H
#define NETRPC_NET_ABSTRACT_PROTOCOL_H

#include <memory>
#include <string>

namespace netrpc {

struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> {
public:
    using AbstractProtocolPtr = std::shared_ptr<AbstractProtocol>;

    virtual ~AbstractProtocol() {}

public:
    std::string m_msg_id; // 请求号，唯一标识一个请求或响应
};    
}

#endif