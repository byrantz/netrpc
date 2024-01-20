#include "netrpc/net/tcp/tcp_server.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/common/log.h"

namespace netrpc {

TcpServer::TcpServer(NetAddr::NetAddrPtr local_addr) : m_local_addr(local_addr) {
    init();

    INFOLOG("netrpc TcpServer listen sucess on [%s]", m_local_addr->toString().c_str());
}

TcpServer::~TcpServer() {
    if (m_main_eventloop) {
        delete m_main_eventloop;
        m_main_eventloop = NULL;
    }
}

void TcpServer::init() {

    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

    m_main_eventloop = EventLoop::GetCurrentEventLoop();
    m_io_thread_groups = new IOThreadGroup(2);

    m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    m_main_eventloop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
    int client_fd = m_acceptor->accept();
    m_client_counts ++;

    INFOLOG("TcpServer succ get client, fd=%d", client_fd);
}

void TcpServer::start() {
    m_io_thread_groups->start();
    m_main_eventloop->loop();
}
}