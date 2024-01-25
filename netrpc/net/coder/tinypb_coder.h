#ifndef NETRPC_NET_CODER_TINYPB_CODER_H
#define NETRPC_NET_CODER_TINYPB_CODER_H

#include "netrpc/net/coder/abstract_coder.h"
#include "netrpc/net/coder/tinypb_protocol.h"

namespace netrpc {

class TinyPBCoder : public AbstractCoder {

public:
    TinyPBCoder() {}
    ~TinyPBCoder() {}

    // 将 message 对象转化为字节流，写入到 buffer
    void encode(std::vector<AbstractProtocol::AbstractProtocolPtr>& messages, TcpBuffer::TcpBufferPtr out_buffer);

    // 将 buffer 里面的字节流转换为 message 对象
    void decode(std::vector<AbstractProtocol::AbstractProtocolPtr>& out_messages, TcpBuffer::TcpBufferPtr buffer);

private:
    const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len);
};

}


#endif