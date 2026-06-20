#pragma once

#include <memory>
#include <string>
#include <vector>

#include "repositories/IAllocationRepository.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/ITimesheetRepository.h"
#include "services/interfaces/INotificationService.h"
#include "services/interfaces/ITimesheetService.h"
#include "validators/TimesheetValidator.h"

class TimesheetService : public ITimesheetService
{
   public:
    TimesheetService(std::shared_ptr<ITimesheetRepository> timesheetRepository,
                     std::shared_ptr<IEmployeeRepository> employeeRepository,
                     std::shared_ptr<IAllocationRepository> allocationRepository,
                     std::shared_ptr<INotificationService> notificationService = nullptr);

    bool submitTimesheet(int employeeId, const std::string& weekStartDate,
                         const std::vector<TimesheetLineInput>& lines, int maxWeeklyHours,
                         std::string& message) override;
    std::vector<Timesheet> getEmployeeTimesheets(int employeeId) override;
    std::vector<TeamTimesheetRow> getTeamTimesheets(int managerUserId,
                                                     const std::string& weekStartDate) override;
    std::vector<TimesheetDetailRow> getTimesheetDetails(int timesheetId) override;
    std::vector<int> getMissedTimesheetEmployeeIds(const std::string& weekStartDate) override;

   private:
    std::string computeWeekEndDate(const std::string& weekStartDate) const;

    std::shared_ptr<ITimesheetRepository>  timesheetRepository_;
    std::shared_ptr<IEmployeeRepository>   employeeRepository_;
    std::shared_ptr<IAllocationRepository> allocationRepository_;
    std::shared_ptr<INotificationService>  notificationService_;
    TimesheetValidator                     timesheetValidator_;
};
