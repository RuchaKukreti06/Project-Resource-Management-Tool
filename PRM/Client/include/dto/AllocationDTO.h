#pragma once
#include <string>
#include <nlohmann/json.hpp>
struct CreateAllocationRequest {
    int employeeId;
    int projectId;
    int utilizationPercentage;
    std::string fromDate;
    std::string toDate;

    nlohmann::json toJson() const {
        return {
            {"employee_id", employeeId},
            {"project_id", projectId},
            {"utilization_percentage", utilizationPercentage},
            {"from_date", fromDate},
            {"to_date", toDate}
        };
    }
};

struct AllocationDTO
{
    int id;
    int employeeId;
    int projectId;
    int utilizationPercentage;
    std::string fromDate;
    std::string toDate;
    
    std::string projectName;

    static AllocationDTO fromJson(const nlohmann::json& j)
    {
        AllocationDTO a;
        a.id = j.value("id", 0);
        a.employeeId = j.value("employee_id", 0);
        a.projectId = j.value("project_id", 0);
        a.utilizationPercentage = j.value("utilization_percentage", 0);
        a.fromDate = j.value("from_date", "");
        a.toDate = j.value("to_date", "");
        
        a.projectName = j.value("project_name", "");
        return a;
    }
};
