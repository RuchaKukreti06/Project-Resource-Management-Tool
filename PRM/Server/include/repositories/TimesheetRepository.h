#pragma once

#include <string>
#include <vector>
#include <optional>
#include "database/Database.h"
#include "repositories/ITimesheetRepository.h"

class TimesheetRepository : public ITimesheetRepository
{
   public:
    explicit TimesheetRepository(database::Database& database);

    bool existsTimesheetForWeek(int employeeId, const std::string& weekStartDate) override;
    bool createTimesheetWithLines(int employeeId, const std::string& weekStartDate,
                                  const std::vector<TimesheetLineInput>& lines) override;
    std::vector<Allocation> getActiveAllocationsForWeek(int employeeId,
                                                         const std::string& weekStartDate,
                                                         const std::string& weekEndDate) override;
    std::optional<Timesheet> getTimesheetById(int timesheetId) override;
    std::vector<Timesheet> getTimesheetsByEmployee(int employeeId) override;
    std::vector<TeamTimesheetRow> getTeamTimesheets(int managerUserId,
                                                     const std::string& weekStartDate) override;
    int getProjectHoursForWeek(int projectId, const std::string& weekStartDate) override;
    std::vector<TimesheetDetailRow> getTimesheetDetails(int timesheetId) override;
    std::vector<int> getEmployeesWithMissedTimesheets(const std::string& weekStartDate) override;

   private:
    database::Database& database_;
};
