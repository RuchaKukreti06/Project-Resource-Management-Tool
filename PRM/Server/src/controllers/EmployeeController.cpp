#include "controllers/EmployeeController.h"

#include <nlohmann/json.hpp>

namespace
{

nlohmann::json makeResponse(bool success, const std::string& message)
{
    return {{"success", success}, {"message", message}};
}

#include "dto/DTOMapper.h"


}  // namespace

EmployeeController::EmployeeController(IEmployeeService& employeeService)
    : employeeService_(employeeService)
{
}

void EmployeeController::registerRoutes(httplib::Server& server)
{
    server.Get("/employees", [this](const httplib::Request& req, httplib::Response& res)
               { handleGetAllEmployees(req, res); });

    server.Get(R"(/managers/(\d+)/team)", [this](const httplib::Request& req, httplib::Response& res)
               { handleGetTeamEmployees(req, res); });

    server.Post("/employees", [this](const httplib::Request& req, httplib::Response& res)
                { handleCreateEmployee(req, res); });

    server.Put(R"(/employees/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
               { handleUpdateEmployee(req, res); });

    server.Patch(R"(/employees/(\d+)/deactivate)",
                 [this](const httplib::Request& req, httplib::Response& res)
                 { handleDeactivateEmployee(req, res); });

    server.Get(R"(/employees/(\d+)/skills)",
               [this](const httplib::Request& req, httplib::Response& res)
               { handleGetEmployeeSkills(req, res); });

    server.Post(R"(/employees/(\d+)/skills)",
                [this](const httplib::Request& req, httplib::Response& res)
                { handleAddSkill(req, res); });

    server.Put(R"(/employees/(\d+)/skills/(\d+))",
               [this](const httplib::Request& req, httplib::Response& res)
               { handleUpdateSkill(req, res); });

    server.Delete(R"(/employees/(\d+)/skills/(\d+))",
                  [this](const httplib::Request& req, httplib::Response& res)
                  { this->handleRemoveSkill(req, res); });
}

void EmployeeController::handleGetAllEmployees(const httplib::Request&, httplib::Response& res)
{
    try
    {
        const auto employees = employeeService_.getAllEmployees();

        res.status = 200;
        res.set_content(nlohmann::json({{"success", true}, {"data", employees}}).dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 500;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleGetTeamEmployees(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const int managerId = std::stoi(req.matches[1]);
        const auto employees = employeeService_.getTeamEmployees(managerId);

        res.status = 200;
        res.set_content(nlohmann::json({{"success", true}, {"data", employees}}).dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 500;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleCreateEmployee(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const auto body = nlohmann::json::parse(req.body);

        EmployeeCreateRequest req;
        req.userId = body.at("user_id").get<int>();
        req.fullName = body.at("full_name").get<std::string>();
        req.email = body.at("email").get<std::string>();
        req.department = body.value("department", "");
        req.designation = body.value("designation", "");

        std::string message;
        const bool ok = employeeService_.createEmployeeProfile(req, message);

        res.status = ok ? 201 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleUpdateEmployee(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const int employeeId = std::stoi(req.matches[1]);
        const auto body = nlohmann::json::parse(req.body);

        Employee employee;
        employee.id = employeeId;
        employee.fullName = body.value("full_name", "");
        employee.email = body.value("email", "");
        employee.department = body.value("department", "");
        employee.designation = body.value("designation", "");
        employee.status = body.value("status", "BENCH");
        employee.isActive = body.value("is_active", true);

        std::string message;
        const bool ok = employeeService_.updateEmployeeProfile(employee, message);

        res.status = ok ? 200 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleDeactivateEmployee(const httplib::Request& req,
                                                  httplib::Response& res)
{
    try
    {
        const int employeeId = std::stoi(req.matches[1]);

        std::string message;
        const bool ok = employeeService_.deactivateEmployee(employeeId, "", message);

        res.status = ok ? 200 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleGetEmployeeSkills(const httplib::Request& req,
                                                 httplib::Response& res)
{
    try
    {
        const int employeeId = std::stoi(req.matches[1]);
        const auto skills = employeeService_.getSkills(employeeId);

        res.status = 200;
        res.set_content(
            nlohmann::json({{"success", true}, {"data", skills}}).dump(),
            "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 500;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleAddSkill(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const auto body = nlohmann::json::parse(req.body);
        AddSkillRequest request;
        request.employeeId = std::stoi(req.matches[1]);
        request.skillName = body.at("skill_name").get<std::string>();
        request.category = body.at("category").get<std::string>();
        request.proficiency = body.at("proficiency").get<std::string>();

        std::string message;
        const bool ok =
            employeeService_.addSkill(request, message);

        res.status = ok ? 201 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleUpdateSkill(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const auto body = nlohmann::json::parse(req.body);
        UpdateSkillRequest request;
        request.employeeId = std::stoi(req.matches[1]);
        request.skillId = std::stoi(req.matches[2]);
        request.proficiency = body.at("proficiency").get<std::string>();

        std::string message;
        const bool ok = employeeService_.updateSkill(request, message);

        res.status = ok ? 200 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleRemoveSkill(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const int employeeId = std::stoi(req.matches[1]);
        const int skillId = std::stoi(req.matches[2]);

        std::string message;
        const bool ok = employeeService_.removeSkill(employeeId, skillId, message);

        res.status = ok ? 200 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}