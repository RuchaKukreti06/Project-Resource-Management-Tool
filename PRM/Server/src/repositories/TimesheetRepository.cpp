#include "repositories/TimesheetRepository.h"

#include <mysqlx/xdevapi.h>
#include <spdlog/spdlog.h>

namespace
{

Timesheet mapTimesheetRow(const mysqlx::Row& row)
{
    Timesheet ts;
    ts.id = row[0].get<int>();
    ts.employeeId = row[1].get<int>();
    ts.weekStartDate = row[2].get<std::string>();
    ts.submittedAt = row[3].isNull() ? "" : row[3].get<std::string>(); // created_at
    ts.status = row[4].get<std::string>();
    return ts;
}

Allocation mapAllocationRow(const mysqlx::Row& row)
{
    Allocation a;
    a.id = row[0].get<int>();
    a.employeeId = row[1].get<int>();
    a.projectId = row[2].get<int>();
    a.utilizationPercentage = row[3].get<int>();
    a.fromDate = row[4].get<std::string>();
    a.toDate = row[5].isNull() ? "" : row[5].get<std::string>();
    return a;
}

int getLastInsertId(mysqlx::Session& session)
{
    auto result = session.sql("SELECT LAST_INSERT_ID()").execute();
    auto row = result.fetchOne();
    return row ? row.get(0) : 0;
}

}  // namespace

TimesheetRepository::TimesheetRepository(database::Database& database) : database_(database)
{
}

bool TimesheetRepository::existsTimesheetForWeek(int employeeId, const std::string& weekStartDate)
{
    try
    {
        auto result =
            database_.getSchema()
                .getTable("timesheets")
                .select("id")
                .where("resource_id = :employee_id AND week_start_date = :week_start_date")
                .bind("employee_id", employeeId)
                .bind("week_start_date", weekStartDate)
                .execute();

        return result.fetchOne().isNull() == false;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("existsTimesheetForWeek failed: {}", e.what());
        return false;
    }
}

bool TimesheetRepository::createTimesheetWithLines(int employeeId, const std::string& weekStartDate,
                                                   const std::vector<TimesheetLineInput>& lines)
{
    auto& session = database_.getSession();

    try
    {
        session.startTransaction();

        database_.getSchema()
            .getTable("timesheets")
            .insert("resource_id", "week_start_date", "status")
            .values(employeeId, weekStartDate, "SUBMITTED")
            .execute();

        const int timesheetId = getLastInsertId(session);
        if (timesheetId <= 0)
        {
            session.rollback();
            return false;
        }

        for (const auto& line : lines)
        {
            database_.getSchema()
                .getTable("timesheet_entries")
                .insert("timesheet_id", "project_id", "hours")
                .values(timesheetId, line.projectId, line.hoursWorked)
                .execute();

            const int lineId = getLastInsertId(session);
            if (lineId <= 0)
            {
                session.rollback();
                return false;
            }

            for (const auto& tag : line.tags)
            {
                database_.getSchema()
                    .getTable("activity_tags")
                    .insert("timesheet_entry_id", "tag_name")
                    .values(lineId, tag)
                    .execute();
            }
        }

        session.commit();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("createTimesheetWithLines failed: {}", e.what());
        try
        {
            session.rollback();
        }
        catch (...)
        {
        }
        return false;
    }
}

std::vector<Allocation> TimesheetRepository::getActiveAllocationsForWeek(
    int employeeId, const std::string& weekStartDate, const std::string& weekEndDate)
{
    std::vector<Allocation> allocations;
    try
    {
        auto result = database_.getSchema()
                          .getTable("allocations")
                          .select("id", "resource_id", "project_id", "utilisation_percent",
                                  "from_date", "to_date")
                          .where(
                              "resource_id = :employee_id AND from_date <= :week_end AND "
                              "(to_date IS NULL OR to_date >= :week_start)")
                          .bind("employee_id", employeeId)
                          .bind("week_end", weekEndDate)
                          .bind("week_start", weekStartDate)
                          .execute();

        while (auto row = result.fetchOne())
        {
            allocations.push_back(mapAllocationRow(row));
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getActiveAllocationsForWeek failed: {}", e.what());
    }

    return allocations;
}

std::optional<Timesheet> TimesheetRepository::getTimesheetById(int timesheetId)
{
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT t.id, t.resource_id, DATE_FORMAT(t.week_start_date, '%Y-%m-%d'), DATE_FORMAT(t.created_at, '%Y-%m-%d %H:%i:%s'), t.status, COALESCE(SUM(te.hours), 0) "
                               "FROM timesheets t "
                               "LEFT JOIN timesheet_entries te ON t.id = te.timesheet_id "
                               "WHERE t.id = ? "
                               "GROUP BY t.id")
                          .bind(timesheetId)
                          .execute();

        if (auto row = result.fetchOne())
        {
            Timesheet ts;
            ts.id = row[0].get<int>();
            ts.employeeId = row[1].get<int>();
            ts.weekStartDate = row[2].get<std::string>();
            ts.submittedAt = row[3].isNull() ? "" : row[3].get<std::string>();
            ts.status = row[4].get<std::string>();
            ts.totalHours = row[5].get<int>();
            return ts;
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getTimesheetById failed: {}", e.what());
    }
    return std::nullopt;
}

std::vector<Timesheet> TimesheetRepository::getTimesheetsByEmployee(int employeeId)
{
    std::vector<Timesheet> timesheets;
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT t.id, t.resource_id, DATE_FORMAT(t.week_start_date, '%Y-%m-%d'), DATE_FORMAT(t.created_at, '%Y-%m-%d %H:%i:%s'), t.status, COALESCE(SUM(te.hours), 0) "
                               "FROM timesheets t "
                               "LEFT JOIN timesheet_entries te ON t.id = te.timesheet_id "
                               "WHERE t.resource_id = ? "
                               "GROUP BY t.id "
                               "ORDER BY t.week_start_date DESC")
                          .bind(employeeId)
                          .execute();

        for (auto row : result)
        {
            Timesheet ts;
            ts.id = row[0].get<int>();
            ts.employeeId = row[1].get<int>();
            ts.weekStartDate = row[2].get<std::string>();
            ts.submittedAt = row[3].isNull() ? "" : row[3].get<std::string>();
            ts.status = row[4].get<std::string>();
            ts.totalHours = row[5].get<int>();
            timesheets.push_back(ts);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getTimesheetsByEmployee failed: {}", e.what());
    }

    return timesheets;
}

std::vector<TimesheetDetailRow> TimesheetRepository::getTimesheetDetails(int timesheetId)
{
    std::vector<TimesheetDetailRow> details;
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT p.id, p.name, te.hours, GROUP_CONCAT(at.tag_name SEPARATOR ', ') "
                               "FROM timesheet_entries te "
                               "JOIN projects p ON p.id = te.project_id "
                               "LEFT JOIN activity_tags at ON at.timesheet_entry_id = te.id "
                               "WHERE te.timesheet_id = ? "
                               "GROUP BY te.id")
                          .bind(timesheetId)
                          .execute();

        for (auto row : result)
        {
            TimesheetDetailRow detail;
            detail.projectId = row[0].get<int>();
            detail.projectName = row[1].get<std::string>();
            detail.hours = row[2].get<int>();
            detail.tags = row[3].isNull() ? "" : row[3].get<std::string>();
            details.push_back(detail);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getTimesheetDetails failed: {}", e.what());
    }
    return details;
}

std::vector<TeamTimesheetRow> TimesheetRepository::getTeamTimesheets(
    int managerUserId, const std::string& weekStartDate)
{
    std::vector<TeamTimesheetRow> rows;

    try
    {
        auto result = database_.getSession()
                          .sql(
                              "SELECT u.full_name, r.id, p.id, p.name, IFNULL(te.hours, 0), "
                              "IF(t.id IS NULL, 'MISSED', t.status) "
                              "FROM projects p "
                              "JOIN allocations a ON a.project_id = p.id "
                              "JOIN resources r ON r.id = a.resource_id "
                              "JOIN users u ON u.id = r.user_id "
                              "LEFT JOIN timesheets t ON t.resource_id = r.id "
                              "AND t.week_start_date = ? "
                              "LEFT JOIN timesheet_entries te ON te.timesheet_id = t.id "
                              "AND te.project_id = p.id "
                              "WHERE p.manager_id = ?")
                          .bind(weekStartDate)
                          .bind(managerUserId)
                          .execute();

        for (auto row : result)
        {
            TeamTimesheetRow view;
            view.employeeName = row[0].get<std::string>();
            view.employeeId = row[1].get<int>();
            view.projectId = row[2].get<int>();
            view.projectName = row[3].get<std::string>();
            view.hours = row[4].get<int>();
            view.status = row[5].get<std::string>();
            rows.push_back(view);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getTeamTimesheets failed: {}", e.what());
    }

    return rows;
}

int TimesheetRepository::getProjectHoursForWeek(int projectId, const std::string& weekStartDate)
{
    try
    {
        auto result = database_.getSession()
                          .sql(
                              "SELECT COALESCE(SUM(te.hours), 0) "
                              "FROM timesheet_entries te "
                              "JOIN timesheets t ON t.id = te.timesheet_id "
                              "WHERE te.project_id = ? AND t.week_start_date = ?")
                          .bind(projectId)
                          .bind(weekStartDate)
                          .execute();

        if (auto row = result.fetchOne())
        {
            return row.get(0);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getProjectHoursForWeek failed: {}", e.what());
    }

    return 0;
}

std::vector<int> TimesheetRepository::getEmployeesWithMissedTimesheets(
    const std::string& weekStartDate)
{
    std::vector<int> ids;
    try
    {
        // Employees who have an active allocation during that week but no timesheet submitted
        auto result = database_.getSession()
                          .sql(
                              "SELECT DISTINCT r.user_id "
                              "FROM resources r "
                              "JOIN users u ON u.id = r.user_id "
                              "JOIN allocations a ON a.resource_id = r.id "
                              "WHERE a.from_date <= ? AND (a.to_date IS NULL OR a.to_date >= ?) "
                              "AND u.is_active = 1 "
                              "AND r.user_id NOT IN ("
                              "  SELECT r2.user_id FROM timesheets t "
                              "  JOIN resources r2 ON r2.id = t.resource_id "
                              "  WHERE t.week_start_date = ? AND t.status = 'SUBMITTED')")
                          .bind(weekStartDate)
                          .bind(weekStartDate)
                          .bind(weekStartDate)
                          .execute();

        for (auto row : result)
        {
            ids.push_back(row[0].get<int>());
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getEmployeesWithMissedTimesheets failed: {}", e.what());
    }
    return ids;
}
