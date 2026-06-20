#include "controllers/AIController.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "utils/ConfigLoader.h"
#include "exceptions/Exceptions.h"

namespace
{
nlohmann::json makeResponse(bool success, const std::string& message)
{
    return {{"success", success}, {"message", message}};
}

nlohmann::json parseLLMResponse(const std::string& raw)
{
    std::string cleaned = raw;
    auto fence = cleaned.find("```");
    if (fence != std::string::npos)
    {
        auto newline = cleaned.find('\n', fence);
        if (newline != std::string::npos)
            cleaned = cleaned.substr(newline + 1);
        auto closing = cleaned.rfind("```");
        if (closing != std::string::npos)
            cleaned = cleaned.substr(0, closing);
        while (!cleaned.empty() && (cleaned.front() == '\n' || cleaned.front() == '\r' || cleaned.front() == ' '))
            cleaned.erase(cleaned.begin());
        while (!cleaned.empty() && (cleaned.back() == '\n' || cleaned.back() == '\r' || cleaned.back() == ' '))
            cleaned.pop_back();
    }

    try
    {
        return nlohmann::json::parse(cleaned);
    }
    catch (...)
    {
        spdlog::warn("AI JSON parse failed, returning raw text");
        return {{"raw", raw}};
    }
}
}  // namespace

AIController::AIController(std::shared_ptr<AIService> aiService, std::shared_ptr<ISystemConfigRepository> configRepo)
    : aiService_(std::move(aiService)), configRepo_(std::move(configRepo))
{
}

void AIController::registerRoutes(httplib::Server& server)
{
    server.Post("/ai/skill-match",
                [this](const httplib::Request& req, httplib::Response& res)
                { handleSkillMatch(req, res); });

    server.Post("/ai/risk-summary",
                [this](const httplib::Request& req, httplib::Response& res)
                { handleRiskSummary(req, res); });

    server.Post("/ai/team-builder",
                [this](const httplib::Request& req, httplib::Response& res)
                { handleTeamBuilder(req, res); });
}

void AIController::handleSkillMatch(const httplib::Request& req, httplib::Response& res)
{
    const auto body        = nlohmann::json::parse(req.body);
    const auto requirement = body.at("requirement").get<std::string>();

    auto cfg = configRepo_->getConfig();
    const auto apiKey  = cfg.llmApiKey;
    const auto provider = cfg.llmProvider;
    const int maxHrs   = cfg.maxWeeklyHours;

    const std::string result = aiService_->skillMatch(requirement, maxHrs, apiKey, provider);

    if (result.empty())
    {
        spdlog::error("AI skill-match: LLM returned empty response (check API key/provider)");
        throw exceptions::AppException("AI service returned no response. Please verify the API key in System Configuration.");
    }

    nlohmann::json parsed = parseLLMResponse(result);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"data", parsed}}).dump(),
                    "application/json");
}

void AIController::handleRiskSummary(const httplib::Request& req, httplib::Response& res)
{
    const auto body      = nlohmann::json::parse(req.body);
    const int  projectId = body.at("project_id").get<int>();
    const auto today     = body.value("today_date", std::string(""));

    auto cfg = configRepo_->getConfig();
    const auto apiKey  = cfg.llmApiKey;
    const auto provider = cfg.llmProvider;

    // Compute today's date if not provided
    std::string effectiveDate = today;
    if (effectiveDate.empty())
    {
        std::time_t now = std::time(nullptr);
        std::tm tm      = {};
#ifdef _WIN32
        localtime_s(&tm, &now);
#else
        tm = *std::localtime(&now);
#endif
        char buf[11] = {};
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
        effectiveDate = buf;
    }

    const std::string summary =
        aiService_->riskSummary(projectId, effectiveDate, apiKey, provider);

    if (summary.empty())
    {
        spdlog::error("AI risk-summary: LLM returned empty response (check API key/provider)");
        throw exceptions::AppException("AI service returned no response. Please verify the API key in System Configuration.");
    }

    res.status = 200;
    res.set_content(
        nlohmann::json({{"success", true}, {"data", {{"summary", summary}}}}).dump(),
        "application/json");
}

void AIController::handleTeamBuilder(const httplib::Request& req, httplib::Response& res)
{
    const auto body        = nlohmann::json::parse(req.body);
    const auto requirement = body.at("requirement").get<std::string>();

    auto cfg = configRepo_->getConfig();
    const auto apiKey  = cfg.llmApiKey;
    const auto provider = cfg.llmProvider;

    const std::string result = aiService_->teamBuilder(requirement, apiKey, provider);

    if (result.empty())
    {
        spdlog::error("AI team-builder: LLM returned empty response");
        throw exceptions::AppException("AI service returned no response. Please verify the API key in System Configuration.");
    }

    nlohmann::json parsed = parseLLMResponse(result);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"data", parsed}}).dump(),
                    "application/json");
}
