#pragma once

#include <string>

struct SystemConfig
{
    int         id;
    std::string llmProvider;
    std::string llmApiKey;
    int         schedulerIntervalHrs;
    int         maxWeeklyHours;
    bool        smtpEnabled;
    std::string smtpHost;
    int         smtpPort;
    std::string smtpUsername;
    std::string smtpPassword;
    std::string smtpFromEmail;
    std::string smtpFromName;
    bool        smtpUseTls;
};

class ISystemConfigRepository
{
   public:
    virtual ~ISystemConfigRepository() = default;

    virtual SystemConfig getConfig() = 0;
    virtual bool         updateConfig(const SystemConfig& config) = 0;
};
