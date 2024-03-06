#include "netrpc/common/log.h"
#include "netrpc/common/util.h"
#include "netrpc/common/config.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/common/run_time.h"
#include <sstream>
#include <assert.h>
#include <sys/time.h>
#include <stdio.h>
#include <iostream>


namespace netrpc {

// static Logger* g_logger = NULL;

Logger::Logger(LogLevel level, int type /*=1*/) : m_set_level(level), m_type{type} {
    if (m_type == 0) {
        return;
    }
}

void Logger::Init(int type /*=1*/) {

  LogLevel global_log_level = StringToLogLevel(Config::GetInst().m_log_level);
  printf("Init log level [%s]\n", LogLevelToString(global_log_level).c_str());
  m_type = type;
  m_set_level = global_log_level;
  m_asnyc_logger = std::make_shared<AsyncLogger>(
    Config::GetInst().m_log_file_name + "_rpc",
    Config::GetInst().m_log_file_path,
    Config::GetInst().m_log_max_file_size);

  m_asnyc_app_logger = std::make_shared<AsyncLogger>(
    Config::GetInst().m_log_file_name + "_app",
    Config::GetInst().m_log_file_path,
    Config::GetInst().m_log_max_file_size);
  if (m_type == 0) {
    return;
  }
  m_timer_event = std::make_shared<TimerEvent>(Config::GetInst().m_log_sync_interval, true, std::bind(&Logger::syncLoop, this));
  EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);
}

void Logger::syncLoop() {
    // 同步 m_buffer 到 async_logger 的 buffer 队尾
    // printf("sync to async logger\n");
    std::vector<std::string> tmp_vec;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        tmp_vec.swap(m_buffer);
    }

    if (!tmp_vec.empty()) {
        m_asnyc_logger->pushLogBuffer(tmp_vec);
    }
    tmp_vec.clear();

    // 同步 m_app_buffer 到 app_async_logger 的 buffer 队尾
    std::vector<std::string> tmp_vec2;
    {
        std::lock_guard<std::mutex> lock2(m_app_mutex);
        tmp_vec2.swap(m_app_buffer);
    }

    if (!tmp_vec2.empty()) {
        m_asnyc_app_logger->pushLogBuffer(tmp_vec2);
    }
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

// eg: [INFO]	[24-03-06 16:01:02.116]	[2526464:2526467]	[99998888]	[makeOrder]
std::string LogEvent::toString() {
    struct timeval now_time;

    gettimeofday(&now_time, nullptr);

    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t);

    // 将格式化的字符串存储在 buf 中
    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%d", &now_time_t);
    std::string time_str(buf);
    int ms = now_time.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();

    std::stringstream ss;

    ss << "[" << LogLevelToString(m_level) << "]\t"
        << "[" << time_str << "]\t"
        << "[" << m_pid << ":" << m_thread_id << "]\t";

    // 获取当前线程处理的请求的 msgid
    std::string msgid = RunTime::GetRunTime()->m_msgid;
    std::string method_name = RunTime::GetRunTime()->m_method_name;
    if (!msgid.empty()) {
        ss << "[" << msgid << "]\t";
    }

    if (!method_name.empty()) {
        ss << "[" << method_name << "]\t";
    }

    return ss.str();

}

void Logger::pushLog(const std::string& msg) { 
    // m_type = 0 表示客户端， 1 表示服务端
    if (m_type == 0) {
        printf("%s\n", msg.c_str());
        return;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    m_buffer.push_back(msg);
}

void Logger::pushAppLog(const std::string& msg) {
    std::lock_guard<std::mutex> lock(m_app_mutex);
    m_app_buffer.push_back(msg);
}

void Logger::log() {

}

AsyncLogger::AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size)
    : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size) {

    sem_init(&m_sempahore, 0, 0);
    assert(pthread_create(&m_thread, NULL, &AsyncLogger::Loop, this) == 0);

    sem_wait(&m_sempahore);
}

void* AsyncLogger::Loop(void* arg) {
    // 将 buffer 里面的全部数据打印到文件中，然后线程睡眠，直到有新的数据再重复这个过程

    AsyncLogger* logger = reinterpret_cast<AsyncLogger*>(arg);

    sem_post(&logger->m_sempahore);

    while (1) {
        std::vector<std::string> tmp;
        {
            std::unique_lock<std::mutex> lock(logger->m_mutex);
            while (logger->m_buffer.empty()) {
                logger->m_condvariable.wait(lock);
            }
            tmp.swap(logger->m_buffer.front());
            logger->m_buffer.pop();

            lock.unlock();
        }

        // 获取当前时间和格式，用于生成日志文件名中的日期部分
        timeval now;
        gettimeofday(&now, NULL);

        struct tm now_time;
        localtime_r(&(now.tv_sec), &now_time);

        const char * format = "%Y%m%d";
        // const char * format = "%Y%m{}%H%M{}";

        char date[32];
        strftime(date, sizeof(date), format, &now_time);

        if (std::string(date) != logger->m_date) {
            logger->m_no = 0;
            logger->m_reopen_flag = true;
            logger->m_date = std::string(date);
        }
        if (logger->m_file_handler == NULL) {
            logger->m_reopen_flag = true;
        }

        std::stringstream ss;
        ss << logger->m_file_path << logger->m_file_name << "_" 
            << std::string(date) << "_log.";
        std::string log_file_name = ss.str() + std::to_string(logger->m_no);

        // 如果需要重新打开文件
        if (logger->m_reopen_flag) {
            if (logger->m_file_handler) {
                // 先将文件关闭
                fclose(logger->m_file_handler);
            }
            // 打开新的日志文件
            logger->m_file_handler = fopen(log_file_name.c_str(), "a");
            logger->m_reopen_flag = false; // 重置重新打开文件标志
        }

        // 如果当前日志文件大小超过最大限制，关闭当前文件，创建新的文件
        if (ftell(logger->m_file_handler) > logger->m_max_file_size) {
            fclose(logger->m_file_handler);

            log_file_name == ss.str() + std::to_string(logger->m_no ++);
            logger->m_file_handler = fopen(log_file_name.c_str(), "a");
            logger->m_reopen_flag = false;
        }

        // 将 tmp 中的日志消息写入文件，同时用 fflush 刷新文件缓冲区，确保所有内容都写入磁盘
        for (auto& i : tmp) {
            if (!i.empty()) {
                fwrite(i.c_str(), 1, i.length(), logger->m_file_handler);
            }
        }
        fflush(logger->m_file_handler);

        if (logger->m_stop_flag) {
            return NULL;
        }
    }

    return NULL;
}

void AsyncLogger::stop() {
    m_stop_flag = true;
}

void AsyncLogger::flush() {
    if (m_file_handler) {
        fflush(m_file_handler);
    }
}

void AsyncLogger::pushLogBuffer(std::vector<std::string>& vec) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_buffer.push(vec);
        m_condvariable.notify_one();
    }

    // 这时候需要唤醒异步日志线程
    // printf("pthread_cond_signal\n");
}

} 