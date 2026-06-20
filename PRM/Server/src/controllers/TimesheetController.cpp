#include "controllers/TimesheetController.h"

#include <nlohmann/json.hpp>

namespace
{

#include "dto/DTOMapper.h"

}

TimesheetController::TimesheetController(ITimesheetService& timesheetService,
                                         INotificationService& notificationService)
    : timesheetService_(timesheetService), notificationService_(notificationService)
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
                        SubmitTimesheetRequest request;
                        request.employeeId = body.at("employee_id").get<int>();
                        request.weekStartDate = body.at("week_start_date").get<std::string>();
                        request.maxWeeklyHours = body.value("max_weekly_hours", 40);

                        for (const auto& lineJson : body.at("lines"))
                        {
                            TimesheetLineDto input;
                            input.projectId = lineJson.at("project_id").get<int>();
                            input.hoursWorked = lineJson.at("hours_worked").get<int>();
                            for (const auto& tag : lineJson.value("tags", nlohmann::json::array()))
                            {
                                input.tags.push_back(tag.get<std::string>());
                            }
                            request.lines.push_back(input);
                        }

                        std::string message;
                        const bool ok = timesheetService_.submitTimesheet(request, message);

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

    server.Put("/timesheets/access/restore",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   try
                   {
                       const auto body = nlohmann::json::parse(req.body);
                       std::string message;
                       const bool ok = notificationService_.restoreTimesheetAccess(
                           body.at("user_id").get<int>(),
                           body.at("week_start_date").get<std::string>(),
                           message);

                       res.status = ok ? 200 : 400;
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

                   nlohmann::json response;
                   response["success"] = true;
                   response["data"] = timesheets;
                   res.set_content(response.dump(), "application/json");
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

                   res.set_content(
                       nlohmann::json({{"success", true}, {"data", rows}}).dump(),
                       "application/json");
               });

    server.Get(R"(/timesheets/(\d+))",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   const int timesheetId = std::stoi(req.matches[1]);
                   const auto details = timesheetService_.getTimesheetDetails(timesheetId);

                   res.set_content(
                       nlohmann::json({{"success", true}, {"data", details}}).dump(),
                       "application/json");
               });
}
