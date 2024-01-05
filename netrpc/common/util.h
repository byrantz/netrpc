#ifndef NETRPC_COMMON_UTIL_H
#define NETRPC_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace netrpc {

pid_t getPid();

pid_t getThreadId();


}

#endif