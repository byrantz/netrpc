#ifndef NETRPC_COMMON_MSGID_UTIL_H
#define NETRPC_COMMON_MSGID_UTIL_H

#include <string>

namespace netrpc {

class MsgIDUtil {

public:
    static std::string GenMsgID();
};

}

#endif