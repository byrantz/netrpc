#include "netrpc/common/run_time.h"

namespace netrpc {

thread_local RunTime* t_run_time = NULL;

RunTime* RunTime::GetRunTime() {
    if (t_run_time) {
        return t_run_time;
    }
    t_run_time = new RunTime();
    return t_run_time;
}

RpcInterface* RunTime::getRpcInterface() {
    return m_rpc_interface;
}
}