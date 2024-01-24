#ifndef NETRPC_NET_ABSTRACT_CODER_H
#define NETRPC_NET_ABSTRACT_CODER_H

#include <vector>
#include "netrpc/net/tcp/tcp_buffer.h"
#include "netrpc/net/abstract_protocol.h"

namespace netrpc {

class AbstractCoder {
public:
    // 将 message 对象转化为字节流，写入到 buffer
    virtual void encode(std::vector<AbstractProtocol::AbstractProtocolPtr>& messages, TcpBuffer::TcpBufferPtr out_buffer) = 0;

    // 将 buffer 里面的字节流转换为 message 对象
    virtual void decode(std::vector<AbstractProtocol::AbstractProtocolPtr>& out_messages, TcpBuffer::TcpBufferPtr buffer) = 0;

    virtual ~AbstractCoder() {}
};

}

#endif