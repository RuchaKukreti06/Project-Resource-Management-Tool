#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

class ILLMProvider {
public:
    virtual ~ILLMProvider() = default;
    virtual std::string query(const std::string& prompt, const std::string& apiKey) = 0;
};

#include "repositories/IAllocationRepository.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/IProjectRepository.h"
#include "repositories/ITimesheetRepository.h"

class AIService
{
   public:
    AIService(std::shared_ptr<IEmployeeRepository> employeeRepo,
              std::shared_ptr<IAllocationRepository> allocationRepo,
              std::shared_ptr<IProjectRepository> projectRepo,
              std::shared_ptr<ITimesheetRepository> timesheetRepo);

    /// Natural-language skill-match: returns JSON array of {name, reason, employee_id}
    nlohmann::json skillMatch(const std::string& requirement, int maxWeeklyHours,
                              const std::string& apiKey, const std::string& provider);

    /// Risk summary for a project: returns a plain-English paragraph
    std::string riskSummary(int projectId, const std::string& todayDate,
                            const std::string& apiKey, const std::string& provider);

    /// Team builder: returns JSON array of {role, employee_id, name, reason}
    nlohmann::json teamBuilder(const std::string& requirement, const std::string& apiKey,
                               const std::string& provider);

   private:
    nlohmann::json parseLLMResponse(const std::string& raw);
    std::shared_ptr<ILLMProvider> getProvider(const std::string& providerName);

    std::string buildEmployeeContextForLLM(bool includeFullyAllocated);

    std::shared_ptr<IEmployeeRepository>   employeeRepo_;
    std::shared_ptr<IAllocationRepository> allocationRepo_;
    std::shared_ptr<IProjectRepository>    projectRepo_;
    std::shared_ptr<ITimesheetRepository>  timesheetRepo_;
};
