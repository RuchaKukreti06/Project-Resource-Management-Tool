#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct SystemConfigDTO
{
    std::string llmProvider;
    std::string llmApiKey;
    int schedulerIntervalHours;
    int maxWeeklyHours;
    bool smtpEnabled;
    std::string smtpHost;
    int smtpPort;
    std::string smtpUsername;
    std::string smtpPassword;
    std::string smtpFromEmail;
    std::string smtpFromName;
    bool smtpUseTls;

    static SystemConfigDTO fromJson(const nlohmann::json& j)
    {
        SystemConfigDTO c;
        c.llmProvider = j.value("llm_provider", "Google Gemini");
        c.llmApiKey = j.value("llm_api_key", "");
        c.schedulerIntervalHours = j.value("scheduler_interval", 24);
        c.maxWeeklyHours = j.value("max_weekly_hours", 40);
        c.smtpEnabled = j.value("smtp_enabled", false);
        c.smtpHost = j.value("smtp_host", "");
        c.smtpPort = j.value("smtp_port", 587);
        c.smtpUsername = j.value("smtp_username", "");
        c.smtpPassword = j.value("smtp_password", "");
        c.smtpFromEmail = j.value("smtp_from_email", "");
        c.smtpFromName = j.value("smtp_from_name", "");
        c.smtpUseTls = j.value("smtp_use_tls", true);
        return c;
    }
};
