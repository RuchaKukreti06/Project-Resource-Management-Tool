#ifndef FINANCE_SERVER_UTILS_CONFIGLOADER_H
#define FINANCE_SERVER_UTILS_CONFIGLOADER_H

#include <string>
#include <nlohmann/json.hpp>

namespace utils {

class ConfigLoader {
public:
    static ConfigLoader& instance();
    void load(const std::string& path);
    std::string serverHost() const;
    int serverPort() const;
    std::string logLevel() const;
    std::string logFile() const;
    std::string dbHost() const;
    int dbPort() const;
    std::string dbName() const;
    std::string dbUser() const;
    std::string dbPassword() const;
    std::string jwtSecret() const;
    int jwtExpirationMinutes() const;
    std::string gemmaIp() const;
    void save(const std::string& path);
    const nlohmann::json& raw() const;

private:
    ConfigLoader() = default;
    nlohmann::json config_;
};

}

#endif
