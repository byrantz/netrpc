#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "netrpc/common/log.h"
#include "netrpc/net/tcp/tcp_client.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/net/fd_event_group.h"
#include "netrpc/common/error_code.h"
#include "netrpc/net/tcp/net_addr.h"

namespace netrpc {

TcpClient::TcpClient(NetAddr::NetAddrPtr peer_addr) : m_peer_addr(peer_addr) {
    m_eventloop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);

    if (m_fd < 0) {
        ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
        return;
    }

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();

    m_connection = std::make_shared<TcpConnection>(m_eventloop, m_fd, 128, peer_addr, nullptr, TcpConnectionByClient);
    m_connection->setConnectionType(TcpConnectionByClient);

}

TcpClient::~TcpClient() {
    DEBUGLOG("TcpClient::~TcpClient()");
    if (m_fd > 0) {
        close(m_fd);
    }
}

// 异步的进行 connect
// 如果 connect 成功， done 会被执行
void TcpClient::connect(std::function<void()> done) {
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if (rt == 0) {
        DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
        m_connection->setState(Connected);
        initLocalAddr();
        if (done) {
            done();
        }
    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            // epoll 监听可写事件，然后判断错误码
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done]() {
                int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
            if ((rt < 0 && errno == EISCONN) || (rt == 0)) {
                DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                initLocalAddr();
                m_connection->setState(Connected);
            } else {
                if (errno == ECONNREFUSED) {
                    m_connect_error_code = ERROR_PEER_CLOSED;
                    m_connect_error_info = "connect refused, sys error = " + std::string(strerror(errno));
                } else {
                    m_connect_error_code = ERROR_FAILED_CONNECT;
                    m_connect_error_info = "connect unkonwn error, sys error =" + std::string(strerror(errno));
                }
                ERRORLOG("connect error, errno=%d, error=%s", errno, strerror(errno));
                close(m_fd);
                m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
            }


            // 连接完后需要去掉可写事件的监听，不然会一直触发
            m_eventloop->deleteEpollEvent(m_fd_event);
            DEBUGLOG("now begin to done");
            // 如果连接完成，才会执行回调函数
            if (done) {
                done();
            }
        });

               
        m_eventloop->addEpollEvent(m_fd_event);

        if (!m_eventloop->isLooping()) {
            m_eventloop->loop();
        }
    }   else {
            ERRORLOG("connect error, errno=%d, error=%s", errno, strerror(errno));
            m_connect_error_code = ERROR_FAILED_CONNECT;
            m_connect_error_info = "connect error, sys error = " + std::string(strerror(errno));
            if (done) {
                done();
            }
        }
    }
}

void TcpClient::stop() {
    if (m_eventloop->isLooping()) {
        m_eventloop->stop();
    }
}

// 异步发送 message
void TcpClient::writeMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done) {
    // 1. 把 message 对象写入到 Connection 的 buffer，done 也写入
    // 2. 启动 connection 可写事件
    m_connection->pushSendMessage(message, done);
    m_connection->listenWrite();
}

// 异步的读取 message
// 如果读取 message 成功，会调用 done 函数，函数的入参就是 message 对象
void TcpClient::readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done) {
    // 1. 监听可读事件
    // 2. 从 buffer 里 decode 得到 message 对象，判断是否 msg_id 相等，相等则读成功，执行其回调
    m_connection->pushReadMessage(msg_id, done);
    m_connection->listenRead();
}

int TcpClient::getConnectErrorCode() {
  return m_connect_error_code;
}

std::string TcpClient::getConnectErrorInfo() {
  return m_connect_error_info;

}

NetAddr::NetAddrPtr TcpClient::getPeerAddr() {
  return m_peer_addr;
}

NetAddr::NetAddrPtr TcpClient::getLocalAddr() {
  return m_local_addr;
}

void TcpClient::initLocalAddr() {
  sockaddr_in local_addr;
  socklen_t len = sizeof(local_addr);

  int ret = getsockname(m_fd, reinterpret_cast<sockaddr*>(&local_addr), &len);
  if (ret != 0) {
    ERRORLOG("initLocalAddr error, getsockname error. errno=%d, error=%s", errno, strerror(errno));
    return;
  }

  m_local_addr = std::make_shared<IPNetAddr>(local_addr);

}

void TcpClient::addTimerEvent(TimerEvent::TimerEventPtr timer_event) {
    m_eventloop->addTimerEvent(timer_event);
}

}