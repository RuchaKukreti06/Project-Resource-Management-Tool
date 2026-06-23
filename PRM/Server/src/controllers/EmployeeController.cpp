#include "controllers/EmployeeController.h"

#include <nlohmann/json.hpp>

#include "exceptions/Exceptions.h"

namespace
{

#include "dto/DTOMapper.h"

}  // namespace

EmployeeController::EmployeeController(IEmployeeService& employeeService, ITokenService& tokenService)
    : employeeService_(employeeService), tokenService_(tokenService)
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

void EmployeeController::handleGetAllEmployees(const httplib::Request& req, httplib::Response& res)
{
    std::string authHeader = req.get_header_value("Authorization");
    std::string token = authHeader.substr(7);
    std::string tokenRole = tokenService_.getClaimRole(token);

    if (tokenRole != "ADMIN") {
        throw exceptions::AuthorizationException("Forbidden: Only ADMIN can view the employee directory.");
    }

    const auto employees = employeeService_.getAllEmployees();

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"data", employees}}).dump(),
                    "application/json");
}

void EmployeeController::handleGetTeamEmployees(const httplib::Request& req, httplib::Response& res)
{
    std::string authHeader = req.get_header_value("Authorization");
    std::string token = authHeader.substr(7);
    std::string tokenRole = tokenService_.getClaimRole(token);
    int tokenUserId = tokenService_.getClaimUserId(token);

    const int managerId = std::stoi(req.matches[1]);
    
    if (tokenRole != "ADMIN" && (tokenRole != "MANAGER" || tokenUserId != managerId)) {
        throw exceptions::AuthorizationException("Forbidden: You cannot view this team's employees.");
    }

    const auto employees = employeeService_.getTeamEmployees(managerId);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"data", employees}}).dump(),
                    "application/json");
}

void EmployeeController::handleCreateEmployee(const httplib::Request& req, httplib::Response& res)
{
    const auto body = nlohmann::json::parse(req.body);

    EmployeeCreateRequest createReq;
    createReq.userId = body.at("user_id").get<int>();
    createReq.fullName = body.at("full_name").get<std::string>();
    createReq.email = body.at("email").get<std::string>();
    createReq.department = body.value("department", "");
    createReq.designation = body.value("designation", "");

    employeeService_.createEmployeeProfile(createReq);

    res.status = 201;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Profile created."}}).dump(), "application/json");
}

void EmployeeController::handleUpdateEmployee(const httplib::Request& req, httplib::Response& res)
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

    employeeService_.updateEmployeeProfile(employee);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Employee updated."}}).dump(), "application/json");
}

void EmployeeController::handleDeactivateEmployee(const httplib::Request& req,
                                                  httplib::Response& res)
{
    const int employeeId = std::stoi(req.matches[1]);

    employeeService_.deactivateEmployee(employeeId, "");

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Employee deactivated. Active allocations ended and login blocked."}}).dump(), "application/json");
}

void EmployeeController::handleGetEmployeeSkills(const httplib::Request& req,
                                                 httplib::Response& res)
{
    const int employeeId = std::stoi(req.matches[1]);
    const auto skills = employeeService_.getSkills(employeeId);

    res.status = 200;
    res.set_content(
        nlohmann::json({{"success", true}, {"data", skills}}).dump(),
        "application/json");
}

void EmployeeController::handleAddSkill(const httplib::Request& req, httplib::Response& res)
{
    const auto body = nlohmann::json::parse(req.body);
    AddSkillRequest request;
    request.employeeId = std::stoi(req.matches[1]);
    request.skillName = body.at("skill_name").get<std::string>();
    request.category = body.at("category").get<std::string>();
    request.proficiency = body.at("proficiency").get<std::string>();

    employeeService_.addSkill(request);

    res.status = 201;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Skill added."}}).dump(), "application/json");
}

void EmployeeController::handleUpdateSkill(const httplib::Request& req, httplib::Response& res)
{
    const auto body = nlohmann::json::parse(req.body);
    UpdateSkillRequest request;
    request.employeeId = std::stoi(req.matches[1]);
    request.skillId = std::stoi(req.matches[2]);
    request.proficiency = body.at("proficiency").get<std::string>();

    employeeService_.updateSkill(request);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Skill updated."}}).dump(), "application/json");
}

void EmployeeController::handleRemoveSkill(const httplib::Request& req, httplib::Response& res)
{
    const int employeeId = std::stoi(req.matches[1]);
    const int skillId = std::stoi(req.matches[2]);

    employeeService_.removeSkill(employeeId, skillId);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Skill removed."}}).dump(), "application/json");
}