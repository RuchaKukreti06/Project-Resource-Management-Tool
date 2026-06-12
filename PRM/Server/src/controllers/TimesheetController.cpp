#include "controllers/TimesheetController.h"

#include <nlohmann/json.hpp>

namespace
{

nlohmann::json timesheetToJson(const Timesheet& timesheet)
{
    return {{"id", timesheet.id},
            {"employee_id", timesheet.employeeId},
            {"week_start_date", timesheet.weekStartDate},
            {"submitted_at", timesheet.submittedAt},
            {"status", timesheet.status},
            {"total_hours", timesheet.totalHours}};
}

}

TimesheetController::TimesheetController(TimesheetService& timesheetService)
    : timesheetService_(timesheetService)
{
}

void TimesheetController::registerRoutes(httplib::Server& server) const
{
    server.Post("/timesheets",
                [&](const httplib::Request& req, httplib::Response& res)
                {
                    try
                    {
                        const auto body = nlohmann::json::parse(req.body);
                        std::vector<TimesheetLineInput> lines;

                        for (const auto& lineJson : body.at("lines"))
                        {
                            TimesheetLineInput input;
                            input.projectId = lineJson.at("project_id").get<int>();
                            input.hoursWorked = lineJson.at("hours_worked").get<int>();
                            for (const auto& tag : lineJson.value("tags", nlohmann::json::array()))
                            {
                                input.tags.push_back(tag.get<std::string>());
                            }
                            lines.push_back(input);
                        }

                        std::string message;
                        const bool ok = timesheetService_.submitTimesheet(
                            body.at("employee_id").get<int>(),
                            body.at("week_start_date").get<std::string>(), lines,
                            body.value("max_weekly_hours", 40), message);

                        res.status = ok ? 201 : 400;
                        res.set_content(
                            nlohmann::json({{"success", ok}, {"message", message}}).dump(),
                            "application/json");
                    }
                    catch (const std::exception& e)
                    {
                        res.status = 400;
                        res.set_content(
                            nlohmann::json({{"success", false}, {"message", e.what()}}).dump(),
                            "application/json");
                    }
                });

    server.Get(R"(/employees/(\d+)/timesheets)",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   const int employeeId = std::stoi(req.matches[1]);
                   const auto timesheets = timesheetService_.getEmployeeTimesheets(employeeId);

                   nlohmann::json data = nlohmann::json::array();
                   for (const auto& ts : timesheets)
                   {
                       data.push_back(timesheetToJson(ts));
                   }

                   res.set_content(
                       nlohmann::json({{"success", true}, {"data", data}}).dump(),
                       "application/json");
               });

    server.Get(R"(/managers/(\d+)/timesheets)",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   const int managerId = std::stoi(req.matches[1]);
                   const std::string weekStartDate =
                       req.has_param("week_start_date")
                           ? req.get_param_value("week_start_date")
                           : "";

                   const auto rows = timesheetService_.getTeamTimesheets(managerId, weekStartDate);

                   nlohmann::json data = nlohmann::json::array();
                   for (const auto& row : rows)
                   {
                       data.push_back({{"employee_name", row.employeeName},
                                       {"employee_id", row.employeeId},
                                       {"project_id", row.projectId},
                                       {"project_name", row.projectName},
                                       {"hours", row.hours},
                                       {"status", row.status}});
                   }

                   res.set_content(
                       nlohmann::json({{"success", true}, {"data", data}}).dump(),
                       "application/json");
               });

    server.Get(R"(/timesheets/(\d+))",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   const int timesheetId = std::stoi(req.matches[1]);
                   const auto details = timesheetService_.getTimesheetDetails(timesheetId);

                   nlohmann::json data = nlohmann::json::array();
                   for (const auto& row : details)
                   {
                       data.push_back({
                           {"project_id", row.projectId},
                           {"project_name", row.projectName},
                           {"hours", row.hours},
                           {"tags", row.tags}
                       });
                   }

                   res.set_content(
                       nlohmann::json({{"success", true}, {"data", data}}).dump(),
                       "application/json");
               });
}
