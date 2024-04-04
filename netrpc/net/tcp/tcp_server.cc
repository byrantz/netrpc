#include "netrpc/net/tcp/tcp_server.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/net/tcp/tcp_connection.h"
#include "netrpc/common/log.h"
#include "netrpc/common/config.h"

namespace netrpc {

ZkClient TcpServer::zkCli;

TcpServer::TcpServer(NetAddr::NetAddrPtr local_addr) : m_local_addr(local_addr) {
    init();

    INFOLOG("netrpc TcpServer listen sucess on [%s]", m_local_addr->toString().c_str());
}

TcpServer::~TcpServer() {
    if (m_main_eventloop) {
        delete m_main_eventloop;
        m_main_eventloop = NULL;
    }
    if (m_io_thread_groups) {
        delete m_io_thread_groups;
        m_io_thread_groups = NULL;
    }
    if (m_listen_fd_event) {
        delete m_listen_fd_event;
        m_listen_fd_event = NULL;
    }
}

void TcpServer::init() {

    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

    m_main_eventloop = EventLoop::GetCurrentEventLoop();
    m_io_thread_groups = new IOThreadGroup(Config::GetInst().m_io_threads);

    m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    m_main_eventloop->addEpollEvent(m_listen_fd_event);
    m_clear_client_timer_event = std::make_shared<TimerEvent>(5000, true, std::bind(&TcpServer::ClearClientTimerFunc, this));
    m_main_eventloop->addTimerEvent(m_clear_client_timer_event);

}

void TcpServer::onAccept() {
    // int client_fd = m_acceptor->accept();
    // m_client_counts ++;

    // INFOLOG("TcpServer succ get client, fd=%d", client_fd);
    auto re = m_acceptor->accept();
    int client_fd = re.first;
    NetAddr::NetAddrPtr peer_addr = re.second;

    m_client_counts ++;

    // 把 clientfd 添加到任意 IO 线程里面
    IOThread* io_thread = m_io_thread_groups->getIOThread();
    TcpConnection::TcpConnectionPtr connection = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr, m_local_addr);
    connection->setState(Connected);

    m_client.insert(connection);

    INFOLOG("TcpServer succ get client, fd=%d", client_fd);
}

void TcpServer::start() {
    m_io_thread_groups->start();
    m_main_eventloop->loop();
}

void TcpServer::ClearClientTimerFunc() {
  auto it = m_client.begin();
  for (it = m_client.begin(); it != m_client.end(); ) {
    // TcpConnection::ptr s_conn = i.second;
		// DebugLog << "state = " << s_conn->getState();
    if ((*it) != nullptr && (*it).use_count() > 0 && (*it)->getState() == Closed) {
      // need to delete TcpConnection
      DEBUGLOG("TcpConection [fd:%d] will delete, state=%d", (*it)->getFd(), (*it)->getState());
      it = m_client.erase(it);
    } else {
      it++;
    }

  }

}
}