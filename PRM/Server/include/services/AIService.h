#pragma once

#include <memory>
#include <string>

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
    std::string skillMatch(const std::string& requirement, int maxWeeklyHours,
                           const std::string& apiKey, const std::string& provider);

    /// Risk summary for a project: returns a plain-English paragraph
    std::string riskSummary(int projectId, const std::string& todayDate,
                            const std::string& apiKey, const std::string& provider);

    /// Team builder: returns JSON array of {role, employee_id, name, reason}
    std::string teamBuilder(const std::string& requirement, const std::string& apiKey,
                            const std::string& provider);

   private:
    std::string callGemini(const std::string& prompt, const std::string& apiKey);
    std::string callGroq(const std::string& prompt, const std::string& apiKey);
    std::string callGemmaRemote(const std::string& prompt, const std::string& apiKey);
    std::string callLLM(const std::string& prompt, const std::string& apiKey,
                        const std::string& provider);

    std::string buildEmployeeContextForLLM(bool includeFullyAllocated);

    std::shared_ptr<IEmployeeRepository>   employeeRepo_;
    std::shared_ptr<IAllocationRepository> allocationRepo_;
    std::shared_ptr<IProjectRepository>    projectRepo_;
    std::shared_ptr<ITimesheetRepository>  timesheetRepo_;
};
