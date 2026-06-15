#pragma once

#include <string>

struct SystemConfig
{
    int         id;
    std::string llmProvider;
    std::string llmApiKey;
    int         schedulerIntervalHrs;
    int         maxWeeklyHours;
};

class ISystemConfigRepository
{
   public:
    virtual ~ISystemConfigRepository() = default;

    virtual SystemConfig getConfig() = 0;
    virtual bool         updateConfig(const SystemConfig& config) = 0;
};
