#ifndef NETRPC_COMMON_CONFIG_H
#define NETRPC_COMMON_CONFIG_H

#include <map>
#include <tinyxml/tinyxml.h>

namespace netrpc {

class Config {
public:
    void Init(const char* xmlfile);
    Config(const char* xmlfile);

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    static Config& GetInst() {
        static Config inst;
        return inst;
    }

    ~Config();


public:
    std::string m_log_level{"DEBUG"};
    std::string m_log_file_name;
    std::string m_log_file_path;
    int m_log_max_file_size {0};
    int m_log_sync_interval {0}; // 日志同步间隔，ms

    int m_port {0};
    int m_io_threads {0};

    TiXmlDocument* m_xml_document{NULL};

private:
    Config() = default;
};

}

#endif