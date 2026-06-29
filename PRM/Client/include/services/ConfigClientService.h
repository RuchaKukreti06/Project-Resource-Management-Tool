#ifndef CONFIG_CLIENT_SERVICE_H
#define CONFIG_CLIENT_SERVICE_H

#include "api/IApiClient.h"
#include "dto/SystemConfigDTO.h"
#include "dto/ApiResponse.h"
#include <string>
#include <nlohmann/json.hpp>

class ConfigClientService
{
private:
    IApiClient& apiClient_;

public:
    explicit ConfigClientService(IApiClient& apiClient);

    ApiResponse<SystemConfigDTO> getConfig();
    ApiEmptyResponse updateConfig(const nlohmann::json& patch);
    ApiEmptyResponse sendTestEmail(const std::string& toEmail, const std::string& subject, const std::string& body);
};

#endif // CONFIG_CLIENT_SERVICE_H
