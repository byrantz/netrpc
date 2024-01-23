#ifndef NETRPC_COMMON_LOG_H
#define NETRPC_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <mutex>

#include "netrpc/common/mutex.h"

namespace netrpc {

// 可变参模板函数
template<typename... Args>
std::string formatString(const char* str, Args&&... args) {
    // 获取字符串缓冲区的大小，str + args...
    int size = snprintf(nullptr, 0, str, args...);

    std::string result;
    if (size > 0)
    {
        result.resize(size);
        // 将可变参数和 str 填入 result
        snprintf(&result[0], size + 1, str, args...);
    }

    return result;
}

#define DEBUGLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Debug) \
    { \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Debug)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + netrpc::formatString(str, ##__VA_ARGS__) + "\n"); \
        netrpc::Logger::GetInst().log(); \
    } \

#define INFOLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Info) \
    { \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Info)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + netrpc::formatString(str, ##__VA_ARGS__) + "\n");\
        netrpc::Logger::GetInst().log(); \
    } \

#define ERRORLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Error) \
    { \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Error)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + netrpc::formatString(str, ##__VA_ARGS__) + "\n"); \
        netrpc::Logger::GetInst().log(); \
    } \

    


enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};
/* 1. 提供打印日志的方法 2. 设置日志输出的路径*/
class Logger {
public:
    typedef std::shared_ptr<Logger> s_ptr;

    // Logger(LogLevel level) : m_set_level(level) {}

    void pushLog(const std::string& msg);

    void log();

    LogLevel getLogLevel() const {
        return m_set_level;
    }

public:
    // Singleton, delete copy construct
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    // Singlenton, get object instance
    static Logger& GetInst() {
        static Logger inst;
        return inst;
    }
    

    void Init();

private:
    Logger() = default;
    LogLevel m_set_level;
    std::queue<std::string> m_buffer;

    std::mutex m_mutex;
    
};

std::string LogLevelToString(LogLevel level);

LogLevel StringToLogLevel(const std::string& log_level);

class LogEvent {
public:
    LogEvent(LogLevel level) : m_level(level) {}

    std::string getFileName() const {
        return m_file_name;
    }

    LogLevel getLogLevel() const {
        return m_level;
    }

    std::string toString();

private:
    std::string m_file_name; // 文件名
    int32_t m_file_line; // 行号
    int32_t m_pid; // 进程号
    int32_t m_thread_id; // 线程号

    LogLevel m_level; // 日志级别
};
}

#endif