#include "AllocationController.h"

#include <nlohmann/json.hpp>

namespace
{

#include "dto/DTOMapper.h"

}  // namespace

AllocationController::AllocationController(IAllocationService& allocationService)
    : allocationService_(allocationService)
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
}

void AllocationController::handleCreateAllocation(const httplib::Request& req,
                                                  httplib::Response& res)
{
    const auto body = nlohmann::json::parse(req.body);
    AllocationCreateRequest request;
    request.employeeId = body.at("employee_id").get<int>();
    request.projectId = body.at("project_id").get<int>();
    request.utilizationPercentage = body.at("utilization_percentage").get<int>();
    request.fromDate = body.at("from_date").get<std::string>();
    request.toDate = body.at("to_date").get<std::string>();

    allocationService_.createAllocation(request, /*createdByUserId=*/0);

    res.status = 201;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Allocation created."}}).dump(),
                    "application/json");
}

void AllocationController::handleEndAllocation(const httplib::Request& req, httplib::Response& res)
{
    const auto body = nlohmann::json::parse(req.body);
    EndAllocationRequest request;
    request.allocationId = std::stoi(req.matches[1]);
    request.endDate = body.at("end_date").get<std::string>();

    allocationService_.endAllocation(request);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Allocation ended."}}).dump(),
                    "application/json");
}

void AllocationController::handleGetProjectAllocations(const httplib::Request& req,
                                                       httplib::Response& res)
{
    const int projectId = std::stoi(req.matches[1]);
    const auto allocations = allocationService_.getProjectAllocations(projectId);

    nlohmann::json response;
    response["success"] = true;
    response["data"] = allocations;
    res.status = 200;
    res.set_content(response.dump(), "application/json");
}
