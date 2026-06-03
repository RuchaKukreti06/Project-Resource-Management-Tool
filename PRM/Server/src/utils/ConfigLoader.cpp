#include "utils/ConfigLoader.h"
#include <fstream>
#include <stdexcept>

namespace utils {

ConfigLoader& ConfigLoader::instance() {
    static ConfigLoader inst;
    return inst;
}

void ConfigLoader::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + path);
    }
    config_ = nlohmann::json::parse(file);
}

std::string ConfigLoader::serverHost() const {
    return config_["server"]["host"].get<std::string>();
}

int ConfigLoader::serverPort() const {
    return config_["server"]["port"].get<int>();
}

std::string ConfigLoader::logLevel() const {
    return config_["logging"]["level"].get<std::string>();
}

std::string ConfigLoader::logFile() const {
    return config_["logging"]["file"].get<std::string>();
}

std::string ConfigLoader::dbHost() const {
    return config_["database"]["host"].get<std::string>();
}

int ConfigLoader::dbPort() const {
    return config_["database"]["port"].get<int>();
}

std::string ConfigLoader::dbName() const {
    return config_["database"]["name"].get<std::string>();
}

std::string ConfigLoader::dbUser() const {
    return config_["database"]["user"].get<std::string>();
}

std::string ConfigLoader::dbPassword() const {
    return config_["database"]["password"].get<std::string>();
}

std::string ConfigLoader::jwtSecret() const {
    return config_["jwt"]["secret"].get<std::string>();
}

int ConfigLoader::jwtExpirationMinutes() const {
    return config_["jwt"]["expiration_minutes"].get<int>();
}

const nlohmann::json& ConfigLoader::raw() const {
    return config_;
}

}
