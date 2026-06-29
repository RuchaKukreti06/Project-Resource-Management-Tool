#pragma once
#include <string>

struct AddSkillRequest
{
    int employeeId;
    std::string skillName;
    std::string category;
    std::string proficiency;
};
