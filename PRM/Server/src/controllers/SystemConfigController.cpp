#include "controllers/SystemConfigController.h"
#include <nlohmann/json.hpp>

SystemConfigController::SystemConfigController(std::shared_ptr<ISystemConfigRepository> configRepo)
    : configRepo_(std::move(configRepo))
{
}

void SystemConfigController::registerRoutes(httplib::Server& server)
{
    server.Get("/system/config", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetConfig(req, res);
    });
    server.Put("/system/config", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpdateConfig(req, res);
    });
}

void SystemConfigController::handleGetConfig(const httplib::Request&, httplib::Response& res)
{
    auto sysCfg = configRepo_->getConfig();
    nlohmann::json data = {
        {"llm_provider",        sysCfg.llmProvider},
        {"llm_api_key",         sysCfg.llmApiKey.empty() ? "" : "****"},
        {"scheduler_interval",  sysCfg.schedulerIntervalHrs},
        {"max_weekly_hours",    sysCfg.maxWeeklyHours},
        {"smtp_enabled",        sysCfg.smtpEnabled},
        {"smtp_host",           sysCfg.smtpHost},
        {"smtp_port",           sysCfg.smtpPort},
        {"smtp_username",       sysCfg.smtpUsername},
        {"smtp_password",       sysCfg.smtpPassword.empty() ? "" : "****"},
        {"smtp_from_email",     sysCfg.smtpFromEmail},
        {"smtp_from_name",      sysCfg.smtpFromName},
        {"smtp_use_tls",        sysCfg.smtpUseTls}
    };
    res.set_content(
        nlohmann::json({{"success", true}, {"data", data}}).dump(),
        "application/json");
}

void SystemConfigController::handleUpdateConfig(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const auto body = nlohmann::json::parse(req.body);
        auto sysCfg = configRepo_->getConfig();

        if (body.contains("llm_api_key"))
            sysCfg.llmApiKey = body["llm_api_key"].get<std::string>();
        if (body.contains("llm_provider"))
            sysCfg.llmProvider = body["llm_provider"].get<std::string>();
        if (body.contains("max_weekly_hours"))
            sysCfg.maxWeeklyHours = body["max_weekly_hours"].get<int>();
        if (body.contains("scheduler_interval"))
            sysCfg.schedulerIntervalHrs = body["scheduler_interval"].get<int>();
        if (body.contains("smtp_enabled"))
            sysCfg.smtpEnabled = body["smtp_enabled"].get<bool>();
        if (body.contains("smtp_host"))
            sysCfg.smtpHost = body["smtp_host"].get<std::string>();
        if (body.contains("smtp_port"))
            sysCfg.smtpPort = body["smtp_port"].get<int>();
        if (body.contains("smtp_username"))
            sysCfg.smtpUsername = body["smtp_username"].get<std::string>();
        if (body.contains("smtp_password"))
            sysCfg.smtpPassword = body["smtp_password"].get<std::string>();
        if (body.contains("smtp_from_email"))
            sysCfg.smtpFromEmail = body["smtp_from_email"].get<std::string>();
        if (body.contains("smtp_from_name"))
            sysCfg.smtpFromName = body["smtp_from_name"].get<std::string>();
        if (body.contains("smtp_use_tls"))
            sysCfg.smtpUseTls = body["smtp_use_tls"].get<bool>();

        configRepo_->updateConfig(sysCfg);

        res.set_content(
            nlohmann::json({{"success", true},
                            {"message", "Configuration updated."}}).dump(),
            "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(
            nlohmann::json({{"success", false}, {"message", e.what()}}).dump(),
            "application/json");
    }
}
