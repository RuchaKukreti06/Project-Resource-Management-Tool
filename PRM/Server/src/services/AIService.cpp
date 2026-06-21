#include "services/AIService.h"

#include <ctime>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <algorithm>
#include <cctype>

#include "utils/DateUtils.h"
#include "utils/ConfigLoader.h"

AIService::AIService(std::shared_ptr<IEmployeeRepository>   employeeRepo,
                     std::shared_ptr<IAllocationRepository> allocationRepo,
                     std::shared_ptr<IProjectRepository>    projectRepo,
                     std::shared_ptr<ITimesheetRepository>  timesheetRepo)
    : employeeRepo_(std::move(employeeRepo)),
      allocationRepo_(std::move(allocationRepo)),
      projectRepo_(std::move(projectRepo)),
      timesheetRepo_(std::move(timesheetRepo))
{
}

// ─── LLM BACKENDS ────────────────────────────────────────────────────────────

nlohmann::json AIService::parseLLMResponse(const std::string& raw)
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

class GeminiProvider : public ILLMProvider {
public:
    std::string query(const std::string& prompt, const std::string& apiKey) override {
        httplib::SSLClient cli("generativelanguage.googleapis.com");
        cli.set_connection_timeout(15);
        cli.set_read_timeout(30);

        nlohmann::json body = {
            {"contents", {{{"parts", {{{"text", prompt}}}}}}}
        };

        const std::string path =
            "/v1beta/models/gemini-1.5-flash:generateContent?key=" + apiKey;

        auto res = cli.Post(path, body.dump(), "application/json");
        if (!res || res->status != 200)
        {
            spdlog::error("Gemini API call failed. Status: {}", res ? res->status : -1);
            return "";
        }

        auto respJson = nlohmann::json::parse(res->body);
        return respJson["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();
    }
};

class GroqProvider : public ILLMProvider {
public:
    std::string query(const std::string& prompt, const std::string& apiKey) override {
        httplib::SSLClient cli("api.groq.com");
        cli.set_connection_timeout(15);
        cli.set_read_timeout(30);

        nlohmann::json body = {
            {"model",    "llama3-8b-8192"},
            {"messages", {{{"role", "user"}, {"content", prompt}}}}
        };

        httplib::Headers headers = {{"Authorization", "Bearer " + apiKey}};
        auto res = cli.Post("/openai/v1/chat/completions", headers, body.dump(), "application/json");

        if (!res || res->status != 200)
        {
            spdlog::error("Groq API call failed. Status: {}", res ? res->status : -1);
            return "";
        }

        auto respJson = nlohmann::json::parse(res->body);
        return respJson["choices"][0]["message"]["content"].get<std::string>();
    }
};

class GemmaRemoteProvider : public ILLMProvider {
public:
    std::string query(const std::string& prompt, const std::string& apiKey) override {
        httplib::Client cli(utils::ConfigLoader::instance().gemmaIp());
        cli.set_connection_timeout(15);
        cli.set_read_timeout(60);

        httplib::Headers headers;
        if (!apiKey.empty())
        {
            headers.emplace("apikey", apiKey);
        }

        nlohmann::json body = {
            {"model", "gemma3:12b-it-q8_0"},
            {"prompt", prompt},
            {"stream", false}
        };

        auto res = cli.Post("/api/generate", headers, body.dump(), "application/json");

        if (!res || res->status != 200)
        {
            spdlog::error("Gemma (Remote) API call failed. Status: {}", res ? res->status : -1);
            return "";
        }

        auto respJson = nlohmann::json::parse(res->body);
        return respJson.value("response", "");
    }
};

std::shared_ptr<ILLMProvider> AIService::getProvider(const std::string& providerName)
{
    std::string provLower = providerName;
    std::transform(provLower.begin(), provLower.end(), provLower.begin(), ::tolower);

    if (provLower == "gemma (remote)" || provLower == "ollama" || provLower == "ollama (gemma3)")
        return std::make_shared<GemmaRemoteProvider>();
    else if (provLower == "groq")
        return std::make_shared<GroqProvider>();
    
    return std::make_shared<GeminiProvider>();
}

std::string AIService::buildEmployeeContextForLLM(bool includeFullyAllocated)
{
    const std::string today = utils::currentDateIso();
    const auto employees = employeeRepo_->getAllEmployees();
    std::ostringstream context;

    for (const auto& e : employees)
    {
        if (!e.isActive)
            continue;

        const int util     = allocationRepo_->getCurrentUtilization(e.id, today);
        const int freePct  = 100 - util;

        if (!includeFullyAllocated && freePct <= 0)
            continue;

        const auto skills  = employeeRepo_->getEmployeeSkills(e.id);
        
        std::string allocEndStr = "";
        if (util > 0)
        {
            auto overlaps = allocationRepo_->getOverlappingAllocations(e.id, today, "2099-12-31");
            for (const auto& a : overlaps)
            {
                if (a.toDate > allocEndStr) allocEndStr = a.toDate;
            }
        }

        context << "- ID:" << e.id
                << " Name:" << e.fullName
                << " Dept:" << e.department
                << " FreeCapacity:" << freePct << "%"
                << (util > 0 && !allocEndStr.empty() ? (" AllocatedUntil:" + allocEndStr) : "")
                << " Skills:";
        for (const auto& s : skills)
            context << s.skillName << "(" << s.proficiencyLevel << ") ";
        context << "\n";
    }
    return context.str();
}

// ─── SKILL MATCH ─────────────────────────────────────────────────────────────

nlohmann::json AIService::skillMatch(const std::string& requirement, int maxWeeklyHours,
                                   const std::string& apiKey, const std::string& provider)
{
    const std::string today = utils::currentDateIso();

    // Gather all active employees
    const auto employees = employeeRepo_->getAllEmployees();

    std::ostringstream context;
    context << "You are a resource manager AI. Given the requirement and employee data below, "
               "return a JSON array of the top 3 best-matched employees. Each element must have: "
               "\"employee_id\" (int), \"name\" (string), \"reason\" (1-2 sentence string). "
               "Only recommend employees who have at least 10% free capacity.\n\n";

    context << "REQUIREMENT: " << requirement << "\n\n";
    context << "EMPLOYEES:\n" << buildEmployeeContextForLLM(false);

    context << "\nReturn ONLY valid JSON array. No explanation outside the JSON.";

    if (apiKey.empty())
    {
        // Return mock response when no API key is configured
        nlohmann::json mock = nlohmann::json::array();
        for (const auto& e : employees)
        {
            if (!e.isActive) continue;
            const int util = allocationRepo_->getCurrentUtilization(e.id, today);
            if (100 - util <= 0) continue;
            mock.push_back({
                {"employee_id", e.id},
                {"name",        e.fullName},
                {"reason",      "Skills and availability match the requirement. (AI key not configured — showing available employees.)"}
            });
            if (mock.size() >= 3) break;
        }
        return mock.dump();
    }

    auto llm = getProvider(provider);
    return parseLLMResponse(llm->query(context.str(), apiKey));
}

// ─── RISK SUMMARY ────────────────────────────────────────────────────────────

std::string AIService::riskSummary(int projectId, const std::string& todayDate,
                                    const std::string& apiKey, const std::string& provider)
{
    const auto project    = projectRepo_->getProjectById(projectId);
    const auto milestones = projectRepo_->getMilestonesByProject(projectId);
    const auto allocs     = allocationRepo_->getActiveAllocationsByProject(projectId);

    std::ostringstream ctx;
    ctx << "You are a project health analyst AI. Based on the project data below, "
           "write a concise plain-English paragraph (3-5 sentences) summarising the key risks "
           "and recommended actions. Be specific about names and dates.\n\n";

    ctx << "PROJECT: " << project.name
        << " | Status: " << project.status
        << " | End Date: " << project.endDate
        << " | Health: " << project.healthStatus << "\n\n";

    ctx << "MILESTONES:\n";
    for (const auto& m : milestones)
    {
        const bool overdue = m.status != "DONE" && m.dueDate < todayDate;
        ctx << "  - " << m.title
            << " [due " << m.dueDate << "] [" << m.status << "]"
            << (overdue ? " OVERDUE" : "") << "\n";
    }

    ctx << "\nALLOCATED RESOURCES:\n";
    for (const auto& a : allocs)
    {
        const auto emp = employeeRepo_->getEmployeeById(a.employeeId);
        const int hrs  = timesheetRepo_->getProjectHoursForWeek(projectId, todayDate);
        ctx << "  - " << emp.fullName
            << " (" << a.utilizationPercentage << "% util)"
            << " hrs logged last week: " << hrs << "\n";
    }

    ctx << "\nToday's date: " << todayDate;

    if (apiKey.empty())
    {
        // Deterministic fallback summary
        std::string summary = "Project " + project.name + " is currently marked as " +
                              project.healthStatus + ". ";
        for (const auto& m : milestones)
        {
            if (m.status != "DONE" && m.dueDate < todayDate)
                summary += "Milestone '" + m.title + "' is overdue. ";
        }
        summary += "Review allocations and milestone progress with your team.";
        return summary;
    }

    auto llm = getProvider(provider);
    return llm->query(ctx.str(), apiKey);
}

// ─── TEAM BUILDER ────────────────────────────────────────────────────────────

nlohmann::json AIService::teamBuilder(const std::string& requirement, const std::string& apiKey,
                                   const std::string& provider)
{
    const std::string today = utils::currentDateIso();
    const auto employees = employeeRepo_->getAllEmployees();

    std::ostringstream context;
    context << "You are a resource manager AI allocating a full project team.\n"
            << "The user will provide a list of roles they need.\n"
            << "Your job is to search the provided employee database and find the BEST bench employee for each requested role.\n\n"
            << "CRITICAL RULES:\n"
            << "1. Return a JSON array where each object has: \"role\" (string), \"employee_id\" (int), \"name\" (string), and \"reason\" (string).\n"
            << "2. You MUST NOT allocate the same employee to more than one role.\n"
            << "3. Only recommend employees who are on the BENCH (FreeCapacity = 100%).\n"
            << "4. If no one has the required skill, set employee_id to 0, name to \"N/A\", and give a reason like 'Nobody has the required skill (so hire or train)'.\n"
            << "5. If someone has the skill but is allocated elsewhere (FreeCapacity < 100%), set employee_id to 0, name to \"N/A\", and give a reason mentioning they have the skill but are allocated elsewhere until their allocation ends.\n\n";

    context << "REQUIREMENT: " << requirement << "\n\n";
    context << "EMPLOYEES:\n" << buildEmployeeContextForLLM(true);

    context << "\nReturn ONLY valid JSON array. No explanation outside the JSON.";

    if (apiKey.empty())
    {
        // Mock response if no API key
        nlohmann::json mock = nlohmann::json::array();
        mock.push_back({
            {"role", "JAVA Developer"},
            {"employee_id", 0},
            {"name", "N/A"},
            {"reason", "No API key configured to parse team requirement."}
        });
        return mock.dump();
    }

    auto llm = getProvider(provider);
    return parseLLMResponse(llm->query(context.str(), apiKey));
}
