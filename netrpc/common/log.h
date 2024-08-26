#ifndef NETRPC_COMMON_LOG_H
#define NETRPC_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <mutex>
#include <semaphore.h>
#include <condition_variable>
#include <sstream>
#include <cstdio>
#include <stdexcept>
#include <pthread.h>
#include "netrpc/common/config.h"
#include "netrpc/net/timer_event.h"

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
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Debug)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n"); \
    } \

#define INFOLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Info) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Info)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n");\
    } \

#define ERRORLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Error) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Error)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n"); \
    } \

#define APPDEBUGLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Debug) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushAppLog((netrpc::LogEvent(netrpc::LogLevel::Debug)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n"); \
    } \

#define APPINFOLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Info) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushAppLog((netrpc::LogEvent(netrpc::LogLevel::Info)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n");\
    } \

#define APPERRORLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Error) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushAppLog((netrpc::LogEvent(netrpc::LogLevel::Error)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n"); \
    } \



enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};

std::string LogLevelToString(LogLevel level);

LogLevel StringToLogLevel(const std::string& log_level);

class AsyncLogger {
public:
    using AsyncLoggerPtr = std::shared_ptr<AsyncLogger>;
    AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size);

    void stop();

    // 刷新到磁盘
    void flush();

    void pushLogBuffer(std::vector<std::string>& vec);

public:
    static void* Loop(void*);

public:
    pthread_t m_thread;

private:
    // m_file_path/m_file_name_yyyymmdd.0

    std::queue<std::vector<std::string>> m_buffer;

    std::string m_file_name; // 日志输出文件名字
    std::string m_file_path; // 日志输出路径
    int m_max_file_size {0}; // 日志单个文件最大大小，单位为字节

    sem_t m_sempahore;

    // pthread_cond_t m_condition; // 条件变量
    std::condition_variable m_condvariable;
    std::mutex m_mutex;

    std::string m_date; // 当前打印日志的文件日期
    FILE* m_file_handler {NULL}; // 当前打开的日志文件句柄

    bool m_reopen_flag {false};

    int m_no {0}; // 日志文件序号

    bool m_stop_flag {false};

};

/* 1. 提供打印日志的方法 2. 设置日志输出的路径*/
class Logger {
public:
    using LoggerPtr = std::shared_ptr<Logger>;

    Logger(LogLevel level, int type = 1);

    void pushLog(const std::string& msg);

    void pushAppLog(const std::string& msg);

    void log();

    LogLevel getLogLevel() const {
        return m_set_level;
    }

    AsyncLogger::AsyncLoggerPtr getAsyncAppLogger() {
        return m_asnyc_app_logger;
    }

    AsyncLogger::AsyncLoggerPtr getAsyncLogger() {
        return m_asnyc_logger;
    }

    void syncLoop();

    void flush();

public:
    // Singleton, delete copy construct
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    // Singlenton, get object instance
    static Logger& GetInst() {
        static Logger inst;
        return inst;
    }
    

    void Init(int type = 1);

private:
    Logger() = default;
    LogLevel m_set_level;
    std::vector<std::string> m_buffer;

    std::vector<std::string> m_app_buffer;

    std::mutex m_mutex;
    std::mutex m_app_mutex;

    std::string m_file_name;
    std::string m_file_path;
    int m_max_file_size {0};

    AsyncLogger::AsyncLoggerPtr m_asnyc_logger;
    AsyncLogger::AsyncLoggerPtr m_asnyc_app_logger;

    TimerEvent::TimerEventPtr m_timer_event;

    int m_type {0};
    
};


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