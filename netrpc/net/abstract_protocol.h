#ifndef NETRPC_NET_ABSTRACT_PROTOCOL_H
#define NETRPC_NET_ABSTRACT_PROTOCOL_H

#include <memory>

namespace netrpc {

class AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> {
public:
    using AbstractProtocolPtr = std::shared_ptr<AbstractProtocol>;

    std::string getReqId() {
        return m_req_id;
    }

    void setReqId(const std::string& req_id) {
        m_req_id = req_id;
    }

    virtual ~AbstractProtocol() {}

protected:
    std::string m_req_id; // 请求号，唯一标识一个请求或响应
};    
}

#endif