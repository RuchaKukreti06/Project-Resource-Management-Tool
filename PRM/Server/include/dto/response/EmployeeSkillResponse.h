#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct EmployeeSkillResponse
{
    int skillId;
    std::string skillName;
    std::string category;
    std::string proficiency;
};

inline void to_json(nlohmann::json& j, const EmployeeSkillResponse& dto)
{
    j = nlohmann::json{
        {"skill_id", dto.skillId},
        {"skill_name", dto.skillName},
        {"category", dto.category},
        {"proficiency", dto.proficiency}
    };
}
