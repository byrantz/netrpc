#ifndef NETRPC_COMMON_CONFIG_H
#define NETRPC_COMMON_CONFIG_H

#include <map>

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


public:
    std::string m_log_level;
    std::string m_log_file_name;
    std::string m_log_file_path;

private:
    Config() = default;
};

}

#endif