#ifndef NETRPC_COMMON_RUN_TIME_H
#define NETRPC_COMMON_RUN_TIME_H

#include <string>

namespace netrpc {
class RpcInterface;

class RunTime {
public:
    RpcInterface* getRpcInterface();

    static RunTime* GetRunTime();

public:
    std::string m_msgid;
    std::string m_method_name;
    RpcInterface* m_rpc_interface {NULL};

};

}

#endif