#include "netrpc/net/io_thread_group.h"
#include "netrpc/common/util.h"
#include "netrpc/common/log.h"

namespace netrpc {

IOThreadGroup::IOThreadGroup(int size) : m_size(size) {
    double CPU_usage = getCPUUtilization(); 
    double sys_load = getAverageLoad();
    if (CPU_usage > 0.8 || sys_load > 0.8){
        DEBUGLOG("CPU usage is high or system load is high, need to encharge IO grooup size");
        size  = size * 2;
    }
    if (CPU_usage < 0.2 || sys_load < 0.2){
        DEBUGLOG("CPU usage is low or system load is low, need to descrease IO grooup size");
        size  = size / 2;
        if (size < 2){
            size = 2;
        }
    }
    m_io_thread_groups.resize(size);
    for (size_t i = 0; (int)i < size; ++ i) {
        m_io_thread_groups[i] = new IOThread();
    }
}

IOThreadGroup::~IOThreadGroup() {

}

void IOThreadGroup::start() {
    for (size_t i = 0; i < m_io_thread_groups.size(); ++i) {
        m_io_thread_groups[i]->start();
    }
}

void IOThreadGroup::join() {
    for (size_t i = 0; i < m_io_thread_groups.size(); ++i) {
        m_io_thread_groups[i]->join();
    }
}

IOThread* IOThreadGroup::getIOThread() {
    if (m_index == (int)m_io_thread_groups.size() || m_index == -1) {
        m_index = 0;
    }
    return m_io_thread_groups[m_index++];
}

}