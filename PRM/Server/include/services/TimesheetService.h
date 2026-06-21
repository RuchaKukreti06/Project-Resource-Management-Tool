#pragma once

#include <memory>
#include <string>
#include <vector>

#include "repositories/IAllocationRepository.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/ITimesheetRepository.h"
#include "repositories/ISystemConfigRepository.h"
#include "services/interfaces/INotificationService.h"
#include "services/interfaces/ITimesheetService.h"
#include "validators/TimesheetValidator.h"

class TimesheetService : public ITimesheetService
{
   public:
    TimesheetService(std::shared_ptr<ITimesheetRepository> timesheetRepository,
                     std::shared_ptr<IEmployeeRepository> employeeRepository,
                     std::shared_ptr<IAllocationRepository> allocationRepository,
                     std::shared_ptr<INotificationService> notificationService,
                     std::shared_ptr<ISystemConfigRepository> systemConfigRepository = nullptr);

    void submitTimesheet(const SubmitTimesheetRequest& req) override;
    std::vector<TimesheetResponse> getEmployeeTimesheets(int employeeId) override;
    std::vector<TeamTimesheetResponse> getTeamTimesheets(int managerUserId,
                                                      const std::string& weekStartDate) override;
    std::vector<TimesheetDetailResponse> getTimesheetDetails(int timesheetId) override;
    std::vector<int> getMissedTimesheetEmployeeIds(const std::string& weekStartDate) override;

   private:
    std::string computeWeekEndDate(const std::string& weekStartDate) const;

    std::shared_ptr<ITimesheetRepository>  timesheetRepository_;
    std::shared_ptr<IEmployeeRepository>   employeeRepository_;
    std::shared_ptr<IAllocationRepository> allocationRepository_;
    std::shared_ptr<INotificationService>  notificationService_;
    std::shared_ptr<ISystemConfigRepository> systemConfigRepository_;
    TimesheetValidator                     timesheetValidator_;
};
