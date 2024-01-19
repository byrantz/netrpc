#ifndef NETRPC_NET_IO_THREAD_GROUP_H
#define NETRPC_NET_IO_THREAD_GROUP_H

#include <vector>
#include "netrpc/common/log.h"
#include "netrpc/net/io_thread.h"

namespace netrpc {

class IOThreadGroup {
public:
    IOThreadGroup(int size);
    
    ~IOThreadGroup();

    void start();

    void join();

    IOThread* getIOThread();

private:
    int m_size {0};
    std::vector<IOThread*> m_io_thread_groups;

    int m_index {0};
};

}

#endif