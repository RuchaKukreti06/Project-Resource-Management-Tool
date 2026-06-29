#include "controllers/TimesheetController.h"

#include <nlohmann/json.hpp>

namespace
{

#include "dto/DTOMapper.h"

}

#include "exceptions/Exceptions.h"

TimesheetController::TimesheetController(ITimesheetService& timesheetService,
                                         INotificationService& notificationService,
                                         ITokenService& tokenService,
                                         IEmployeeService& employeeService)
    : timesheetService_(timesheetService), notificationService_(notificationService),
      tokenService_(tokenService), employeeService_(employeeService)
{
}

void TimesheetController::registerRoutes(httplib::Server& server) const
{
    server.Post("/timesheets",
                [&](const httplib::Request& req, httplib::Response& res)
                {
                    std::string authHeader = req.get_header_value("Authorization");
                    std::string token = authHeader.substr(7);
                    int tokenUserId = tokenService_.getClaimUserId(token);
                    std::string tokenRole = tokenService_.getClaimRole(token);

                    const auto body = nlohmann::json::parse(req.body);
                    SubmitTimesheetRequest request;
                    request.employeeId = body.at("employee_id").get<int>();
                    request.weekStartDate = body.at("week_start_date").get<std::string>();
                    request.maxWeeklyHours = body.value("max_weekly_hours", 40);

                    auto employeeOpt = employeeService_.getEmployeeById(request.employeeId);
                    if (!employeeOpt.has_value()) {
                        throw exceptions::NotFoundException("Employee not found.");
                    }
                    
                    if (tokenRole != "ADMIN" && employeeOpt->user_id != tokenUserId) {
                        throw exceptions::AuthorizationException("Forbidden: You cannot submit timesheets for this employee.");
                    }


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

                    timesheetService_.submitTimesheet(request);

                    res.status = 201;
                    res.set_content(
                        nlohmann::json({{"success", true}, {"message", "Timesheet submitted."}}).dump(),
                        "application/json");
                });

    server.Put("/timesheets/access/restore",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   std::string authHeader = req.get_header_value("Authorization");
                   std::string token = authHeader.substr(7);
                   int tokenUserId = tokenService_.getClaimUserId(token);
                   std::string tokenRole = tokenService_.getClaimRole(token);

                   const auto body = nlohmann::json::parse(req.body);
                   int targetUserId = 0;
                   if (body.contains("user_id")) {
                       targetUserId = body.at("user_id").get<int>();
                   }

                   if (tokenRole == "ADMIN" || tokenRole == "EMPLOYEE") {
                       throw exceptions::AuthorizationException("Forbidden: Only Managers can restore timesheet access for their team.");
                   }

                   std::optional<Employee> employeeOpt;
                   if (body.contains("employee_id")) {
                       employeeOpt = employeeService_.getEmployeeById(body.at("employee_id").get<int>());
                       if (employeeOpt.has_value()) targetUserId = employeeOpt->user_id;
                   } else {
                       employeeOpt = employeeService_.getEmployeeByUserId(targetUserId);
                   }

                   if (!employeeOpt.has_value()) {
                       throw exceptions::NotFoundException("Employee not found.");
                   }
                   if (employeeOpt->manager_id != tokenUserId) {
                       throw exceptions::AuthorizationException("Forbidden: You do not manage this employee.");
                   }

                   notificationService_.restoreTimesheetAccess(
                       targetUserId,
                       body.at("week_start_date").get<std::string>());

                   res.status = 200;
                   res.set_content(
                       nlohmann::json({{"success", true}, {"message", "Timesheet access restored."}}).dump(),
                       "application/json");
               });

    server.Get(R"(/employees/(\d+)/timesheets)",
               [&](const httplib::Request& req, httplib::Response& res)
                {
                   std::string authHeader = req.get_header_value("Authorization");
                   std::string token = authHeader.substr(7);
                   int tokenUserId = tokenService_.getClaimUserId(token);
                   std::string tokenRole = tokenService_.getClaimRole(token);

                   const int employeeId = std::stoi(req.matches[1]);
                   auto employeeOpt = employeeService_.getEmployeeById(employeeId);
                   if (!employeeOpt.has_value()) {
                       throw exceptions::NotFoundException("Employee not found.");
                   }

                    if (tokenRole == "EMPLOYEE" && employeeOpt->user_id != tokenUserId) {
                        throw exceptions::AuthorizationException("Forbidden: You can only view your own timesheets.");
                    } else if (tokenRole == "MANAGER" && employeeOpt->manager_id != tokenUserId) {
                        throw exceptions::AuthorizationException("Forbidden: You can only view timesheets for your team.");
                    } else if (tokenRole != "ADMIN" && tokenRole != "EMPLOYEE" && tokenRole != "MANAGER") {
                        throw exceptions::AuthorizationException("Forbidden: Invalid role.");
                    }

                   const auto timesheets = timesheetService_.getEmployeeTimesheets(employeeId);

                   nlohmann::json response;
                   response["success"] = true;
                   response["data"] = timesheets;
                   res.set_content(response.dump(), "application/json");
               });

    server.Get(R"(/managers/(\d+)/timesheets)",
               [&](const httplib::Request& req, httplib::Response& res)
                {
                   std::string authHeader = req.get_header_value("Authorization");
                   std::string token = authHeader.substr(7);
                   int tokenUserId = tokenService_.getClaimUserId(token);
                   std::string tokenRole = tokenService_.getClaimRole(token);

                   const int managerId = std::stoi(req.matches[1]);
                   
                   if (tokenRole == "EMPLOYEE" || (tokenRole == "MANAGER" && tokenUserId != managerId)) {
                       throw exceptions::AuthorizationException("Forbidden: You cannot view this team's timesheets.");
                   }

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
                   std::string authHeader = req.get_header_value("Authorization");
                   std::string token = authHeader.substr(7);
                   int tokenUserId = tokenService_.getClaimUserId(token);
                   std::string tokenRole = tokenService_.getClaimRole(token);

                   const int timesheetId = std::stoi(req.matches[1]);
                   
                   auto tsOpt = timesheetService_.getTimesheetById(timesheetId);
                   if (!tsOpt.has_value()) {
                       throw exceptions::NotFoundException("Timesheet not found.");
                   }
                   
                   auto employeeOpt = employeeService_.getEmployeeById(tsOpt->employeeId);
                   if (!employeeOpt.has_value()) {
                       throw exceptions::NotFoundException("Employee for this timesheet not found.");
                   }

                   if (tokenRole == "EMPLOYEE" && employeeOpt->user_id != tokenUserId) {
                       throw exceptions::AuthorizationException("Forbidden: You can only view your own timesheet details.");
                   } else if (tokenRole == "MANAGER" && employeeOpt->manager_id != tokenUserId) {
                       throw exceptions::AuthorizationException("Forbidden: You can only view timesheet details for your team.");
                   } else if (tokenRole != "ADMIN" && tokenRole != "EMPLOYEE" && tokenRole != "MANAGER") {
                       throw exceptions::AuthorizationException("Forbidden: Invalid role.");
                   }

                   const auto details = timesheetService_.getTimesheetDetails(timesheetId);

                   res.set_content(
                       nlohmann::json({{"success", true}, {"data", details}}).dump(),
                       "application/json");
               });
}
