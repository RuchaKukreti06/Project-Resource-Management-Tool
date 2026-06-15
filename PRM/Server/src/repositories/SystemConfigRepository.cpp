#include "repositories/SystemConfigRepository.h"
#include <spdlog/spdlog.h>

SystemConfigRepository::SystemConfigRepository(database::Database& db) : db_(db)
{
}

SystemConfig SystemConfigRepository::getConfig()
{
    const std::string query = "SELECT id, llm_provider, llm_api_key, scheduler_interval_hrs, max_weekly_hours FROM system_config LIMIT 1";
    
    try {
        auto result = db_.getSession().sql(query).execute();

        if (auto row = result.fetchOne())
        {
            return {
                row[0].get<int>(),
                row[1].get<std::string>(),
                row[2].get<std::string>(),
                row[3].get<int>(),
                row[4].get<int>()
            };
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to get system config: {}", e.what());
    }

    // Insert default if it doesn't exist
    SystemConfig defaultConfig = {0, "Google Gemini", "", 24, 40};
    const std::string insertQuery = "INSERT INTO system_config (llm_provider, llm_api_key, scheduler_interval_hrs, max_weekly_hours) VALUES (?, ?, ?, ?)";
    try {
        db_.getSession().sql(insertQuery)
            .bind(defaultConfig.llmProvider)
            .bind(defaultConfig.llmApiKey)
            .bind(defaultConfig.schedulerIntervalHrs)
            .bind(defaultConfig.maxWeeklyHours)
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
    const std::string query = "UPDATE system_config SET llm_provider = ?, llm_api_key = ?, scheduler_interval_hrs = ?, max_weekly_hours = ? WHERE id = ?";
    try {
        db_.getSession().sql(query)
            .bind(config.llmProvider)
            .bind(config.llmApiKey)
            .bind(config.schedulerIntervalHrs)
            .bind(config.maxWeeklyHours)
            .bind(config.id)
            .execute();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to update system config: {}", e.what());
        return false;
    }
}
