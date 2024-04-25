#ifndef NETRPC_COMMON_RUN_TIME_H
#define NETRPC_COMMON_RUN_TIME_H

#include <string>

namespace netrpc {

class RunTime {
public:

    static RunTime* GetRunTime();

public:
    std::string m_msgid;
    std::string m_method_name;

};

}

#endif