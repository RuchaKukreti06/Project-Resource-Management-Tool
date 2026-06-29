#include "repositories/SystemConfigRepository.h"
#include <spdlog/spdlog.h>

namespace
{

void tryExecute(mysqlx::Session& session, const std::string& query)
{
    try
    {
        session.sql(query).execute();
    }
    catch (const std::exception& e)
    {
        std::string errMsg = e.what();
        if (errMsg.find("Duplicate column name") == std::string::npos)
        {
            spdlog::warn("System config schema update skipped: {}", errMsg);
        }
    }
}

}

SystemConfigRepository::SystemConfigRepository(database::Database& db) : db_(db)
{
    ensureSchema();
}

void SystemConfigRepository::ensureSchema()
{
    auto& session = db_.getSession();
    tryExecute(session,
               "ALTER TABLE system_config ADD COLUMN smtp_enabled BOOLEAN NOT NULL DEFAULT FALSE");
    tryExecute(session,
               "ALTER TABLE system_config ADD COLUMN smtp_host VARCHAR(255) NOT NULL DEFAULT ''");
    tryExecute(session,
               "ALTER TABLE system_config ADD COLUMN smtp_port INT NOT NULL DEFAULT 587");
    tryExecute(session,
               "ALTER TABLE system_config ADD COLUMN smtp_username VARCHAR(255) NOT NULL DEFAULT ''");
    tryExecute(session,
               "ALTER TABLE system_config ADD COLUMN smtp_password VARCHAR(255) NOT NULL DEFAULT ''");
    tryExecute(session,
               "ALTER TABLE system_config ADD COLUMN smtp_from_email VARCHAR(255) NOT NULL DEFAULT ''");
    tryExecute(session,
               "ALTER TABLE system_config ADD COLUMN smtp_from_name VARCHAR(255) NOT NULL DEFAULT ''");
    tryExecute(session,
               "ALTER TABLE system_config ADD COLUMN smtp_use_tls BOOLEAN NOT NULL DEFAULT TRUE");
}

SystemConfig SystemConfigRepository::getConfig()
{
    const std::string query =
        "SELECT id, llm_provider, llm_api_key, scheduler_interval_hrs, max_weekly_hours, "
        "smtp_enabled, smtp_host, smtp_port, smtp_username, smtp_password, smtp_from_email, "
        "smtp_from_name, smtp_use_tls "
        "FROM system_config LIMIT 1";
    
    try {
        auto result = db_.getSession().sql(query).execute();

        if (auto row = result.fetchOne())
        {
            return {
                row[0].get<int>(),
                row[1].get<std::string>(),
                row[2].get<std::string>(),
                row[3].get<int>(),
                row[4].get<int>(),
                row[5].get<bool>(),
                row[6].get<std::string>(),
                row[7].get<int>(),
                row[8].get<std::string>(),
                row[9].get<std::string>(),
                row[10].get<std::string>(),
                row[11].get<std::string>(),
                row[12].get<bool>()
            };
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to get system config: {}", e.what());
    }

    // Insert default if it doesn't exist
    SystemConfig defaultConfig = {
        0,
        "Google Gemini",
        "",
        24,
        40,
        false,
        "",
        587,
        "",
        "",
        "",
        "",
        true
    };
    const std::string insertQuery =
        "INSERT INTO system_config "
        "(llm_provider, llm_api_key, scheduler_interval_hrs, max_weekly_hours, smtp_enabled, "
        "smtp_host, smtp_port, smtp_username, smtp_password, smtp_from_email, smtp_from_name, smtp_use_tls) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    try {
        db_.getSession().sql(insertQuery)
            .bind(defaultConfig.llmProvider)
            .bind(defaultConfig.llmApiKey)
            .bind(defaultConfig.schedulerIntervalHrs)
            .bind(defaultConfig.maxWeeklyHours)
            .bind(defaultConfig.smtpEnabled)
            .bind(defaultConfig.smtpHost)
            .bind(defaultConfig.smtpPort)
            .bind(defaultConfig.smtpUsername)
            .bind(defaultConfig.smtpPassword)
            .bind(defaultConfig.smtpFromEmail)
            .bind(defaultConfig.smtpFromName)
            .bind(defaultConfig.smtpUseTls)
            .execute();
        
        // Fetch it back to get the ID
        auto retryResult = db_.getSession().sql(query).execute();
        if (auto row = retryResult.fetchOne()) {
            defaultConfig.id = row[0].get<int>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to insert default system config: {}", e.what());
    }

    return defaultConfig;
}

bool SystemConfigRepository::updateConfig(const SystemConfig& config)
{
    const std::string query =
        "UPDATE system_config SET llm_provider = ?, llm_api_key = ?, scheduler_interval_hrs = ?, "
        "max_weekly_hours = ?, smtp_enabled = ?, smtp_host = ?, smtp_port = ?, smtp_username = ?, "
        "smtp_password = ?, smtp_from_email = ?, smtp_from_name = ?, smtp_use_tls = ? WHERE id = ?";
    try {
        db_.getSession().sql(query)
            .bind(config.llmProvider)
            .bind(config.llmApiKey)
            .bind(config.schedulerIntervalHrs)
            .bind(config.maxWeeklyHours)
            .bind(config.smtpEnabled)
            .bind(config.smtpHost)
            .bind(config.smtpPort)
            .bind(config.smtpUsername)
            .bind(config.smtpPassword)
            .bind(config.smtpFromEmail)
            .bind(config.smtpFromName)
            .bind(config.smtpUseTls)
            .bind(config.id)
            .execute();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to update system config: {}", e.what());
        return false;
    }
}
