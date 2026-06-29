#pragma once
#include <string>

struct UpdateSkillRequest
{
    int employeeId;
    int skillId;
    std::string proficiency;
};
