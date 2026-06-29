#pragma once

#include <memory>
#include <string>

#include "services/interfaces/IAllocationService.h"
#include "services/interfaces/IEmployeeService.h"
#include "services/interfaces/INotificationService.h"
#include "services/interfaces/IProjectService.h"
#include "services/interfaces/ITimesheetService.h"

class SchedulerService
{
   public:
    SchedulerService(std::shared_ptr<IEmployeeService> employeeService,
                     std::shared_ptr<IProjectService> projectService,
                     std::shared_ptr<IAllocationService> allocationService,
                     std::shared_ptr<ITimesheetService> timesheetService,
                     std::shared_ptr<INotificationService> notificationService = nullptr);

    void runRecomputationJob(const std::string& todayDate);
    void recomputeProjectHealth(const std::string& todayDate);
    void flagMissedTimesheets(const std::string& lastMondayDate);

   private:
    std::string computeProjectHealth(int projectId, const std::string& todayDate);

    std::shared_ptr<IEmployeeService> employeeService_;
    std::shared_ptr<IProjectService> projectService_;
    std::shared_ptr<IAllocationService> allocationService_;
    std::shared_ptr<ITimesheetService> timesheetService_;
    std::shared_ptr<INotificationService> notificationService_;
};
