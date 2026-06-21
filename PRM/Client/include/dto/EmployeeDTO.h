#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct SkillDTO
{
    int skillId;
    std::string skillName;
    std::string category;
    std::string proficiency;

    static SkillDTO fromJson(const nlohmann::json& j)
    {
        SkillDTO s;
        s.skillId = j.value("skill_id", 0);
        s.skillName = j.value("skill_name", "");
        s.category = j.value("category", "");
        s.proficiency = j.value("proficiency", "");
        return s;
    }
};

struct EmployeeDTO
{
    int id;
    int userId;
    std::string fullName;
    std::string email;
    std::string department;
    std::string designation;
    std::string status;
    bool isActive;
    int totalUtilisation;

    static EmployeeDTO fromJson(const nlohmann::json& j)
    {
        EmployeeDTO e;
        e.id = j.value("id", 0);
        e.userId = j.value("user_id", 0);
        e.fullName = j.value("full_name", "");
        e.email = j.value("email", "");
        e.department = j.value("department", "");
        e.designation = j.value("designation", "");
        e.status = j.value("status", "");
        e.isActive = j.value("is_active", false);
        e.totalUtilisation = j.value("total_utilisation", 0);
        return e;
    }
};
