#include <unistd.h>
#include "netrpc/common/log.h"
#include "netrpc/net/fd_event_group.h"
#include "netrpc/net/tcp/tcp_connection.h"
#include "netrpc/net/coder/string_coder.h"
#include "netrpc/net/coder/tinypb_coder.h"

namespace netrpc {

TcpConnection::TcpConnection(EventLoop* eventloop, int fd, int buffer_size, NetAddr::NetAddrPtr peer_addr, NetAddr::NetAddrPtr local_addr, TcpConnectionType type /* = TcpConnectionByServer*/) 
    : m_eventloop(eventloop), m_local_addr(local_addr), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd), m_connection_type(type) {

        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
        m_fd_event->setNonBlock();
        
        m_coder = new TinyPBCoder();

        if (m_connection_type == TcpConnectionByServer) {
            listenRead();
        }
}

TcpConnection::~TcpConnection() {
    DEBUGLOG("~TcpConnection");
    if (m_coder) {
        delete m_coder;
        m_coder = NULL;
    }
}

/*
这个函数本身的操作模式是异步的。有一个循环，它不会因为等待数据而阻塞，
它会进行尽可能多的非阻塞读取，直至读完所有数据或 read 返回 EAGAIN。
这意味着，虽然函数可能需要在数据准备就绪时被多次调用，它不会阻塞调用者的执行流程。
*/
void TcpConnection::onRead() {
    // 从 socket 缓冲区，调用系统的 read 函数读取字节, 读入到 in_buffer 里面

    if (m_state != Connected) {
        ERRORLOG("onRead error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    bool is_read_all = false;
    bool is_close = false;
    while (!is_read_all) {
        if (m_in_buffer->writeAble() == 0) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
        }
        int read_count = m_in_buffer->writeAble();
        int write_index = m_in_buffer->writeIndex();

        int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);
        if (rt > 0) {
            m_in_buffer->moveWriteIndex(rt);
            if (rt == read_count) {
                continue;
            }  else if (rt < read_count) {
                is_read_all = true;
                break;
            } 
        } else if (rt == 0) { // 表示已经读取到文件末尾，对面断开连接，read 会返回 0
            is_close = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) { // 非阻塞模式下没有数据可读，等一会可能有
            is_read_all = true;
            break;
        }
    }

    if (is_close) {
        INFOLOG("peer closed, peer addr [%d], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
        clear();
        return;
    }

    if (!is_read_all) {
        ERRORLOG("not read all data");
    }

    excute();
}

void TcpConnection::excute() {
    if (m_connection_type == TcpConnectionByServer) {
        // 将 RPC 请求执行业务逻辑，获取 RPC 响应，再把 RPC 响应发送回去
        std::vector<AbstractProtocol::AbstractProtocolPtr> result;
        std::vector<AbstractProtocol::AbstractProtocolPtr> replay_messages;
        m_coder->decode(result, m_in_buffer);

        std::string msg;
        for (size_t i = 0; i < result.size(); ++ i) {
            // 1. 针对每一个请求，调用 rpc 方法，获取响应 message
            // 2. 将响应 message 放入到缓冲区，监听可写事件回包
            INFOLOG("sucsess get request[%s] from client[%s]", result[i]->m_msg_id.c_str(), m_peer_addr->toString().c_str());
        
            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            // message->m_pb_data = "hello, this is netrpc rpc test data";
            // message->m_msg_id = result[i]->m_msg_id;
            RpcDispatcher::GetInst().dispatch(result[i], message, this);
            replay_messages.emplace_back(message);
        }

        m_coder->encode(replay_messages, m_out_buffer);
        // 启动监听可写事件
        listenWrite();
    } else {
        // 从 buffer 里 decode 得到 message 对象，执行其回调
        std::vector<AbstractProtocol::AbstractProtocolPtr> result;
        m_coder->decode(result, m_in_buffer);

        for (size_t i = 0; i < result.size(); ++ i) {
            std::string msg_id = result[i]->m_msg_id;
            auto it = m_read_dones.find(msg_id);
            if (it != m_read_dones.end()) {
                it->second(result[i]);
                m_read_dones.erase(it);
            }
        }
    }

}


void TcpConnection::reply(std::vector<AbstractProtocol::AbstractProtocolPtr>& replay_messages) {
    m_coder->encode(replay_messages, m_out_buffer);
    listenWrite();
}

void TcpConnection::onWrite() {
    // 将当前 out_buffer 里面的数据全部发送给 client

    if (m_state != Connected) {
        ERRORLOG("onWrite error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    if (m_connection_type == TcpConnectionByClient) {
        // 1. 将 message encode 得到字节流
        // 2. 将字节流放入到 buffer 里面，然后全部发送

        std::vector<AbstractProtocol::AbstractProtocolPtr> messages;

        for (size_t i = 0; i < m_write_dones.size(); ++ i) {
            messages.push_back(m_write_dones[i].first);
        }

        m_coder->encode(messages, m_out_buffer);
    }

    bool is_write_all = false;
    while (true) {
        if (m_out_buffer->readAble() == 0) {
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();

        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);

        if (rt >= write_size) {
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            // 发送缓冲区已经满了，不能再发送了
            // 这种情况，需要等下次 fd 可写的时候再次发送数据即可
            ERRORLOG("write data error, errno==EAGAIN and rt == -1");
            break;
        }
    }

    if (is_write_all) {
        m_fd_event->cancle(FdEvent::OUT_EVENT);
        m_eventloop->addEpollEvent(m_fd_event);
    }

    if (m_connection_type == TcpConnectionByClient ) {
        for (size_t i = 0; i < m_write_dones.size(); ++ i) {
            m_write_dones[i].second(m_write_dones[i].first);
        }
        m_write_dones.clear();
    }
}

void TcpConnection::setState(const TcpState state) {
    m_state = Connected;
}

TcpState TcpConnection::getState() {
    return m_state;
}

void TcpConnection::clear() {
    // 处理一些关闭连接后的清理动作
    if (m_state == Closed) {
        return;
    }
    m_fd_event->cancle(FdEvent::IN_EVENT);
    m_fd_event->cancle(FdEvent::OUT_EVENT);
    m_eventloop->deleteEpollEvent(m_fd_event);

    m_state = Closed;
}

void TcpConnection::shutdown() {
    if (m_state == Closed || m_state == NotConnected) {
        return;
    }

    // 处于半关闭
    m_state = HalfClosing;

    // 调用 shutdown 关闭读写，意味着服务器不会再对这个 fd 进行读写操作了
    // 发送 FIN 报文，触发了四次挥手的第一个阶段
    // 当 fd 发生可读事件，但是可读的数据为 0，即 对端发送了 FIN
    ::shutdown(m_fd, SHUT_RDWR);
}

void TcpConnection::setConnectionType(TcpConnectionType type) {
    m_connection_type = type;
}

void TcpConnection::listenWrite() {
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_eventloop->addEpollEvent(m_fd_event);
}

void TcpConnection::listenRead() {
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    m_eventloop->addEpollEvent(m_fd_event);
}

void TcpConnection::pushSendMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done) {
    m_write_dones.push_back(std::make_pair(message, done));
}

void TcpConnection::pushReadMessage(const std::string& msg_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done) {
    m_read_dones.insert(std::make_pair(msg_id, done));
}

NetAddr::NetAddrPtr TcpConnection::getLocalAddr() {
    return m_local_addr;
}

NetAddr::NetAddrPtr TcpConnection::getPeerAddr() {
    return m_peer_addr;
}

int TcpConnection::getFd() {
    return m_fd;
}

}