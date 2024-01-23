#ifndef NETRPC_NET_FD_EVENT_GROUP_H
#define NETRPC_NET_FD_EVENT_GROUP_H

#include <vector>
#include "netrpc/net/fd_event.h"
#include <mutex>

namespace netrpc {
class FdEventGroup {
public:
    FdEventGroup(int size);

    ~FdEventGroup();
    FdEvent* getFdEvent(int fd);

public:
    static FdEventGroup* GetFdEventGroup();

private:
    int m_size {0};
    std::vector<FdEvent*> m_fd_group;
    std::mutex m_mutex;

};

}

#endif