#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct AllocationResponse
{
    int id;
    int employeeId;
    int projectId;
    int utilizationPercentage;
    std::string fromDate;
    std::string toDate;
};

inline void to_json(nlohmann::json& j, const AllocationResponse& dto)
{
    j = nlohmann::json{
        {"id", dto.id},
        {"employee_id", dto.employeeId},
        {"project_id", dto.projectId},
        {"utilization_percentage", dto.utilizationPercentage},
        {"from_date", dto.fromDate},
        {"to_date", dto.toDate}
    };
}
