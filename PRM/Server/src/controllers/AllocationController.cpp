#include "AllocationController.h"

#include <nlohmann/json.hpp>
#include "exceptions/Exceptions.h"

namespace
{

#include "dto/DTOMapper.h"

}  // namespace

AllocationController::AllocationController(IAllocationService& allocationService, ITokenService& tokenService, IEmployeeService& employeeService, IProjectService& projectService)
    : allocationService_(allocationService), tokenService_(tokenService), employeeService_(employeeService), projectService_(projectService)
{
}

void AllocationController::registerRoutes(httplib::Server& server)
{
    server.Post("/allocations", [this](const httplib::Request& req, httplib::Response& res)
                { this->handleCreateAllocation(req, res); });

    server.Put(R"(/allocations/(\d+)/end)",
               [this](const httplib::Request& req, httplib::Response& res)
               { this->handleEndAllocation(req, res); });

    server.Get(R"(/projects/(\d+)/allocations)",
               [this](const httplib::Request& req, httplib::Response& res)
               { this->handleGetProjectAllocations(req, res); });

    server.Get(R"(/employees/(\d+)/allocations)",
               [this](const httplib::Request& req, httplib::Response& res)
               { this->handleGetEmployeeAllocations(req, res); });
}

void AllocationController::handleCreateAllocation(const httplib::Request& req,
                                                  httplib::Response& res)
{
    std::string authHeader = req.get_header_value("Authorization");
    std::string token = authHeader.substr(7);
    int tokenUserId = tokenService_.getClaimUserId(token);
    std::string tokenRole = tokenService_.getClaimRole(token);

    const auto body = nlohmann::json::parse(req.body);
    AllocationCreateRequest request;
    request.employeeId = body.at("employee_id").get<int>();
    request.projectId = body.at("project_id").get<int>();
    request.utilizationPercentage = body.at("utilization_percentage").get<int>();
    request.fromDate = body.at("from_date").get<std::string>();
    request.toDate = body.at("to_date").get<std::string>();

    auto projectOpt = projectService_.getProjectById(request.projectId);
    if (!projectOpt.has_value()) {
        throw exceptions::NotFoundException("Project not found.");
    }
    
    if (tokenRole != "ADMIN" && projectOpt->managerId != tokenUserId) {
        throw exceptions::AuthorizationException("Forbidden: Only the project manager can allocate resources.");
    }

    allocationService_.createAllocation(request, /*createdByUserId=*/0);

    res.status = 201;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Allocation created."}}).dump(),
                    "application/json");
}

void AllocationController::handleEndAllocation(const httplib::Request& req, httplib::Response& res)
{
    std::string authHeader = req.get_header_value("Authorization");
    std::string token = authHeader.substr(7);
    int tokenUserId = tokenService_.getClaimUserId(token);
    std::string tokenRole = tokenService_.getClaimRole(token);

    const auto body = nlohmann::json::parse(req.body);
    EndAllocationRequest request;
    request.allocationId = std::stoi(req.matches[1]);
    request.endDate = body.at("end_date").get<std::string>();

    auto allocationOpt = allocationService_.getAllocationById(request.allocationId);
    if (!allocationOpt.has_value()) {
        throw exceptions::NotFoundException("Allocation not found.");
    }

    auto projectOpt = projectService_.getProjectById(allocationOpt->projectId);
    if (!projectOpt.has_value()) {
        throw exceptions::NotFoundException("Project not found.");
    }

    if (tokenRole != "ADMIN" && projectOpt->managerId != tokenUserId) {
        throw exceptions::AuthorizationException("Forbidden: Only the project manager can end allocations.");
    }

    allocationService_.endAllocation(request);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Allocation ended."}}).dump(),
                    "application/json");
}

void AllocationController::handleGetProjectAllocations(const httplib::Request& req,
                                                       httplib::Response& res)
{
    std::string authHeader = req.get_header_value("Authorization");
    std::string token = authHeader.substr(7);
    int tokenUserId = tokenService_.getClaimUserId(token);
    std::string tokenRole = tokenService_.getClaimRole(token);

    const int projectId = std::stoi(req.matches[1]);
    
    auto projectOpt = projectService_.getProjectById(projectId);
    if (!projectOpt.has_value()) {
        throw exceptions::NotFoundException("Project not found.");
    }

    if (tokenRole == "EMPLOYEE") {
        throw exceptions::AuthorizationException("Forbidden: Employees cannot view project allocations.");
    }
    if (tokenRole == "MANAGER" && projectOpt->managerId != tokenUserId) {
        throw exceptions::AuthorizationException("Forbidden: You can only view allocations for your own projects.");
    }
    const auto allocations = allocationService_.getProjectAllocations(projectId);

    nlohmann::json response;
    response["success"] = true;
    response["data"] = allocations;
    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void AllocationController::handleGetEmployeeAllocations(const httplib::Request& req,
                                                        httplib::Response& res)
{
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.find("Bearer ") != 0)
    {
        throw exceptions::AuthenticationException("Missing or invalid authorization header.");
    }

    std::string token = authHeader.substr(7);
    if (!tokenService_.validateToken(token))
    {
        throw exceptions::AuthenticationException("Invalid token.");
    }

    const int requestedEmployeeId = std::stoi(req.matches[1]);
    const int tokenUserId = tokenService_.getClaimUserId(token);
    const std::string tokenRole = tokenService_.getClaimRole(token);

    auto employeeOpt = employeeService_.getEmployeeById(requestedEmployeeId);
    if (!employeeOpt.has_value())
    {
        throw exceptions::NotFoundException("Employee not found.");
    }

    if (tokenRole == "EMPLOYEE" && employeeOpt->user_id != tokenUserId)
    {
        throw exceptions::AuthorizationException("Not authorized to view another employee's allocations.");
    }
    if (tokenRole == "MANAGER" && employeeOpt->manager_id != tokenUserId)
    {
        throw exceptions::AuthorizationException("Not authorized to view allocations for an employee not in your team.");
    }

    const auto allocations = allocationService_.getEmployeeAllocations(requestedEmployeeId);

    nlohmann::json response;
    response["success"] = true;
    response["data"] = allocations;
    res.status = 200;
    res.set_content(response.dump(), "application/json");
}
