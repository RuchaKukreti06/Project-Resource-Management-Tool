#include "services/ConfigClientService.h"

ConfigClientService::ConfigClientService(IApiClient& apiClient)
    : apiClient_(apiClient)
{
}

ApiResponse<SystemConfigDTO> ConfigClientService::getConfig()
{
    return ApiResponse<SystemConfigDTO>::fromJson(apiClient_.get("/system/config"));
}

ApiEmptyResponse ConfigClientService::updateConfig(const nlohmann::json& patch)
{
    return ApiEmptyResponse::fromJson(apiClient_.put("/system/config", patch));
}

ApiEmptyResponse ConfigClientService::sendTestEmail(const std::string& toEmail, const std::string& subject, const std::string& body)
{
    return ApiEmptyResponse::fromJson(apiClient_.post("/notifications/test-email", {
        {"to_email", toEmail},
        {"subject", subject},
        {"body", body}
    }));
}
