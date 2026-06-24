#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct TimesheetEntryDTO
{
    int id;
    int projectId;
    int hoursWorked;
    std::string tags;
    std::string projectName;

    static TimesheetEntryDTO fromJson(const nlohmann::json& j)
    {
        TimesheetEntryDTO e;
        e.id = j.value("id", 0);
        e.projectId = j.value("project_id", 0);
        e.hoursWorked = j.value("hours", j.value("hours_worked", 0));
        e.tags = j.value("tags", "");
        e.projectName = j.value("project_name", "");
        return e;
    }
};

struct TimesheetDTO
{
    int id;
    int employeeId;
    std::string weekStartDate;
    std::string submittedAt;
    std::string status;
    int totalHours;

    static TimesheetDTO fromJson(const nlohmann::json& j)
    {
        TimesheetDTO t;
        t.id = j.value("id", 0);
        t.employeeId = j.value("employee_id", 0);
        t.weekStartDate = j.value("week_start_date", "");
        t.submittedAt = j.value("submitted_at", "");
        t.status = j.value("status", "");
        t.totalHours = j.value("total_hours", 0);
        return t;
    }
};

struct ManagerTimesheetDTO
{
    std::string employeeName;
    std::string projectName;
    int hours;
    std::string status;

    static ManagerTimesheetDTO fromJson(const nlohmann::json& j)
    {
        ManagerTimesheetDTO m;
        m.employeeName = j.value("employee_name", "");
        m.projectName = j.value("project_name", "");
        m.hours = j.value("hours", 0);
        m.status = j.value("status", "");
        return m;
    }
};

struct TimesheetLineRequest
{
    int projectId;
    int hoursLogged;
    std::string activityTag;

    nlohmann::json toJson() const
    {
        return {
            {"project_id", projectId},
            {"hours_logged", hoursLogged},
            {"activity_tag", activityTag}
        };
    }
};

struct TimesheetCreateRequest
{
    std::vector<TimesheetLineRequest> entries;

    nlohmann::json toJson() const
    {
        nlohmann::json jEntries = nlohmann::json::array();
        for (const auto& e : entries)
        {
            jEntries.push_back(e.toJson());
        }
        return {
            {"entries", jEntries}
        };
    }
};

struct SubmitTimesheetRequest
{
    int employeeId;
    std::string weekStartDate;
    int maxWeeklyHours = 40;
    std::vector<TimesheetLineRequest> lines;

    nlohmann::json toJson() const
    {
        nlohmann::json jLines = nlohmann::json::array();
        for (const auto& l : lines)
        {
            jLines.push_back(l.toJson());
        }
        return {
            {"employee_id", employeeId},
            {"week_start_date", weekStartDate},
            {"max_weekly_hours", maxWeeklyHours},
            {"lines", jLines}
        };
    }
};
