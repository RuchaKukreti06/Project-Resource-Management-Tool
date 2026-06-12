#include "controllers/EmployeeController.h"

#include <nlohmann/json.hpp>

namespace
{

nlohmann::json makeResponse(bool success, const std::string& message)
{
    return {{"success", success}, {"message", message}};
}

nlohmann::json employeeToJson(const Employee& e)
{
    return {{"id",                e.id},
            {"user_id",          e.user_id},
            {"full_name",        e.fullName},
            {"email",            e.email},
            {"department",       e.department},
            {"designation",      e.designation},
            {"status",           e.status},
            {"is_active",        e.isActive},
            {"total_utilisation",e.totalUtilisation}};  // D7
}

nlohmann::json skillToJson(const EmployeeSkillView& s)
{
    return {{"skill_id", s.skillId},
            {"skill_name", s.skillName},
            {"category", s.category},
            {"proficiency", s.proficiencyLevel}};
}

}  // namespace

EmployeeController::EmployeeController(EmployeeService& employeeService)
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

        nlohmann::json data = nlohmann::json::array();
        for (const auto& e : employees) data.push_back(employeeToJson(e));

        res.status = 200;
        res.set_content(nlohmann::json({{"success", true}, {"data", data}}).dump(),
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

        nlohmann::json data = nlohmann::json::array();
        for (const auto& e : employees) data.push_back(employeeToJson(e));

        res.status = 200;
        res.set_content(nlohmann::json({{"success", true}, {"data", data}}).dump(),
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

        const int userId = body.at("user_id").get<int>();
        const std::string fullName = body.at("full_name").get<std::string>();
        const std::string email = body.at("email").get<std::string>();
        const std::string department = body.value("department", "");
        const std::string designation = body.value("designation", "");

        std::string message;
        const bool ok = employeeService_.createEmployeeProfile(userId, fullName, email, department,
                                                               designation, message);

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

        nlohmann::json data = nlohmann::json::array();
        for (const auto& s : skills) data.push_back(skillToJson(s));

        res.status = 200;
        res.set_content(nlohmann::json({{"success", true}, {"data", data}}).dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void EmployeeController::handleAddSkill(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const int employeeId = std::stoi(req.matches[1]);
        const auto body = nlohmann::json::parse(req.body);

        const std::string skillName = body.at("skill_name").get<std::string>();
        const std::string category = body.at("category").get<std::string>();
        const std::string proficiency = body.at("proficiency").get<std::string>();

        std::string message;
        const bool ok =
            employeeService_.addSkill(employeeId, skillName, category, proficiency, message);

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
        const int employeeId = std::stoi(req.matches[1]);
        const int skillId = std::stoi(req.matches[2]);
        const auto body = nlohmann::json::parse(req.body);

        const std::string proficiency = body.at("proficiency").get<std::string>();

        std::string message;
        const bool ok = employeeService_.updateSkill(employeeId, skillId, proficiency, message);

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