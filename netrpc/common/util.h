#ifndef NETRPC_COMMON_UTIL_H
#define NETRPC_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace netrpc {

pid_t getPid();

pid_t getThreadId();

int64_t getNowMs();

// 网络字节序转换为主机字节序
int32_t getInt32FromNetByte(const char* buf);
}

double getCPUUtilization();     // 获取CPU利用率

double getAverageLoad();        // 获取系统的平均负载能力

#endif