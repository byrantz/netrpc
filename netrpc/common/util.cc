#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "netrpc/common/util.h"


namespace netrpc {

static int g_pid = 0;

static thread_local int g_thread_id = 0;

pid_t getPid() {
    if (g_pid != 0) {
        return g_pid;
    }
    return getpid();
}

pid_t getThreadId() {
    if (g_thread_id != 0) {
        return g_thread_id;
    } 
    return syscall(SYS_gettid);
}

// 获取当前时间，并转换为毫秒
int64_t getNowMs() {
    timeval val;
    gettimeofday(&val, NULL);

    return val.tv_sec * 1000 + val.tv_usec / 1000;
}

}