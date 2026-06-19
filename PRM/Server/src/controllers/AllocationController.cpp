#include "AllocationController.h"

#include <nlohmann/json.hpp>

namespace
{

nlohmann::json allocationToJson(const Allocation& allocation)
{
    return {{"id", allocation.id},
            {"employee_id", allocation.employeeId},
            {"project_id", allocation.projectId},
            {"utilization_percentage", allocation.utilizationPercentage},
            {"from_date", allocation.fromDate},
            {"to_date", allocation.toDate}};
}

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
    try
    {
        const auto body = nlohmann::json::parse(req.body);
        Allocation allocation;
        allocation.employeeId = body.at("employee_id").get<int>();
        allocation.projectId = body.at("project_id").get<int>();
        allocation.utilizationPercentage = body.at("utilization_percentage").get<int>();
        allocation.fromDate = body.at("from_date").get<std::string>();
        allocation.toDate = body.at("to_date").get<std::string>();

        std::string message;
        const bool ok =
            allocationService_.createAllocation(allocation, /*createdByUserId=*/0, message);

        res.status = ok ? 201 : 400;
        res.set_content(nlohmann::json({{"success", ok}, {"message", message}}).dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(nlohmann::json({{"success", false}, {"message", e.what()}}).dump(),
                        "application/json");
    }
}

void AllocationController::handleEndAllocation(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const int allocationId = std::stoi(req.matches[1]);
        const auto body = nlohmann::json::parse(req.body);
        const std::string endDate = body.at("end_date").get<std::string>();

        std::string message;
        const bool ok = allocationService_.endAllocation(allocationId, endDate, message);

        res.status = ok ? 200 : 400;
        res.set_content(nlohmann::json({{"success", ok}, {"message", message}}).dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(nlohmann::json({{"success", false}, {"message", e.what()}}).dump(),
                        "application/json");
    }
}

void AllocationController::handleGetProjectAllocations(const httplib::Request& req,
                                                       httplib::Response& res)
{
    try
    {
        const int projectId = std::stoi(req.matches[1]);
        const auto allocations = allocationService_.getProjectAllocations(projectId);

        nlohmann::json data = nlohmann::json::array();
        for (const auto& alloc : allocations)
        {
            data.push_back(allocationToJson(alloc));
        }

        // D4: Wrap in {success, data} envelope consistent with the rest of the API
        res.status = 200;
        res.set_content(
            nlohmann::json({{"success", true}, {"data", data}}).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(nlohmann::json({{"success", false}, {"message", e.what()}}).dump(),
                        "application/json");
    }
}
