#include "ApiClient.h"
#include "Application.h"
#include "AuthSession.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::string baseUrl = "http://localhost:8080";
    const std::string configPath = std::filesystem::exists("Client/config/client_config.json")
                                       ? "Client/config/client_config.json"
                                       : "PRM/Client/config/client_config.json";

    if (std::filesystem::exists(configPath)) {
        try {
            std::ifstream file(configPath);
            nlohmann::json cfg = nlohmann::json::parse(file);
            if (cfg.contains("server_url")) {
                baseUrl = cfg["server_url"].get<std::string>();
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse client config: " << e.what() << "\n";
        }
    }

    ApiClient apiClient(baseUrl);
    Application app(baseUrl, apiClient);
    app.run();

    return 0;
}