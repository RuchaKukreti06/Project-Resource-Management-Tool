#pragma once

#include <memory>
#include <string>

#include "services/AllocationService.h"
#include "services/EmployeeService.h"
#include "services/NotificationService.h"
#include "services/ProjectService.h"
#include "services/TimesheetService.h"

class SchedulerService
{
   public:
    SchedulerService(std::shared_ptr<EmployeeService> employeeService,
                     std::shared_ptr<ProjectService> projectService,
                     std::shared_ptr<AllocationService> allocationService,
                     std::shared_ptr<TimesheetService> timesheetService,
                     std::shared_ptr<NotificationService> notificationService = nullptr);

    void runRecomputationJob(const std::string& todayDate);
    void recomputeProjectHealth(const std::string& todayDate);
    void flagMissedTimesheets(const std::string& lastMondayDate);

   private:
    std::string computeProjectHealth(int projectId, const std::string& todayDate);

    std::shared_ptr<EmployeeService> employeeService_;
    std::shared_ptr<ProjectService> projectService_;
    std::shared_ptr<AllocationService> allocationService_;
    std::shared_ptr<TimesheetService> timesheetService_;
    std::shared_ptr<NotificationService> notificationService_;
};
