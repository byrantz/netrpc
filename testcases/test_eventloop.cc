#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include "netrpc/common/log.h"
#include "netrpc/common/config.h"
#include "netrpc/net/fd_event.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/net/timer_event.h"
#include "netrpc/net/io_thread.h"
#include "netrpc/net/io_thread_group.h"

void test_io_thread() {
    // int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (listenfd == -1) {
    //     ERRORLOG("listenfd = -1");
    //     exit(0);
    // }

    // sockaddr_in addr;
    // memset(&addr, 0, sizeof(addr));

    // addr.sin_port = htons(12310);
    // addr.sin_family = AF_INET;
    // inet_aton("127.0.0.1", &addr.sin_addr);

    // int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    // if (rt != 0) {
    //     ERRORLOG("bind error");
    //     exit(1);
    // }

    // rt = listen(listenfd, 100);
    // if (rt != 0) {
    //     ERRORLOG("listen error");
    //     exit(1);
    // }

    // netrpc::FdEvent event(listenfd);
    // event.listen(netrpc::FdEvent::IN_EVENT, [listenfd]() {
    //     sockaddr_in peer_addr;
    //     socklen_t addr_len = sizeof(peer_addr);
    //     memset(&peer_addr, 0, sizeof(peer_addr));
    //     int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

    //     DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    // });

    int i = 0;
    netrpc::TimerEvent::TimerEventPtr timer_event = std::make_shared<netrpc::TimerEvent> (
        2000, true, [&i]() {
            INFOLOG("trigger timer event, count=%d", i++);
        }
    );

    netrpc::IOThreadGroup io_thread_group(1);
    netrpc::IOThread* io_thread = io_thread_group.getIOThread();
    // io_thread->getEventLoop()->addEpollEvent(&event);
    io_thread->getEventLoop()->addTimerEvent(timer_event);
    // netrpc::IOThread* io_thread2 = io_thread_group.getIOThread();
    // io_thread2->getEventLoop()->addTimerEvent(timer_event);

    io_thread_group.start();
    io_thread_group.join();
    
}

int main() {
    netrpc::Config::GetInst().Init(NULL);
    
    netrpc::Logger::GetInst().Init(0);

    test_io_thread();
    // netrpc::EventLoop* eventloop = new netrpc::EventLoop();

    // int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (listenfd == -1) {
    //     ERRORLOG("listenfd = -1");
    //     exit(0);
    // }

    // sockaddr_in addr;
    // memset(&addr, 0, sizeof(addr));

    // addr.sin_port = htons(12310);
    // addr.sin_family = AF_INET;
    // inet_aton("127.0.0.1", &addr.sin_addr);

    // int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    // if (rt != 0) {
    //     ERRORLOG("bind error");
    //     exit(1);
    // }

    // rt = listen(listenfd, 100);
    // if (rt != 0) {
    //     ERRORLOG("listen error");
    //     exit(1);
    // }

    // netrpc::FdEvent event(listenfd);
    // event.listen(netrpc::FdEvent::IN_EVENT, [listenfd]() {
    //     sockaddr_in peer_addr;
    //     socklen_t addr_len = sizeof(peer_addr);
    //     memset(&peer_addr, 0, sizeof(peer_addr));
    //     int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

    //     DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    // });
    // eventloop->addEpollEvent(&event);

    // int i = 0;
    // netrpc::TimerEvent::TimerEventPtr timer_event = std::make_shared<netrpc::TimerEvent> (
    //     100, true, [&i]() {
    //         INFOLOG("trigger timer event, count=%d", i++);
    //     }
    // );

    // eventloop->addTimerEvent(timer_event);
    // eventloop->loop();

    return 0;
}