#include "netrpc/common/log.h"
#include "netrpc/common/util.h"
#include "netrpc/common/config.h"
#include <sstream>
#include <sys/time.h>
#include <stdio.h>

namespace netrpc {

// static Logger* g_logger = NULL;



void Logger::Init() {
  LogLevel global_log_level = StringToLogLevel(Config::GetInst().m_log_level);
  printf("Init log level [%s]\n", LogLevelToString(global_log_level).c_str());
  m_set_level = global_log_level;

}

std::string LogLevelToString(LogLevel level) {
    switch (level)
    {
    case Debug:
        return "DEBUG";

    case Info:
        return "INFO";

    case Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

LogLevel StringToLogLevel(const std::string& log_level) {
    if (log_level == "DEBUG") {
        return Debug;
    } else if (log_level == "INFO") {
        return Info;
    } else if (log_level == "ERROR") {
        return Error;
    } else {
        return Unknown;
    }
}

std::string LogEvent::toString() {
    struct timeval now_time;

    gettimeofday(&now_time, nullptr);

    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t);

    // 将格式化的字符串存储在 buf 中
    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
    std::string time_str(buf);
    int ms = now_time.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();

    std::stringstream ss;

    ss << "[" << LogLevelToString(m_level) << "]\t"
        << "[" << time_str << "]\t"
        << "[" << m_pid << ":" << m_thread_id << "]\t";

    return ss.str();

}

void Logger::pushLog(const std::string& msg) { 
    std::lock_guard<std::mutex> lock(m_mutex);
    m_buffer.push(msg);
}

void Logger::log() {
    // while (!m_buffer.empty()) {
    //     std::string msg = m_buffer.front();
    //     m_buffer.pop();

    //     printf(msg.c_str());
    
    // }
    std::unique_lock<std::mutex> lock(m_mutex);
    std::queue<std::string> tmp;
    m_buffer.swap(tmp);

    lock.unlock();

    while (!tmp.empty()) {
        std::string msg = tmp.front();
        tmp.pop();
        printf(msg.c_str());
    }
}

} 