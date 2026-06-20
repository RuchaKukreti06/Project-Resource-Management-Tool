#include "validators/EmployeeValidator.h"

#include <unordered_set>

bool EmployeeValidator::validateId(int employeeId, std::string& message) const
{
    if (employeeId <= 0)
    {
        message = "Invalid employee id.";
        return false;
    }
    return true;
}

bool EmployeeValidator::validateSkill(const std::string& category,
                                      const std::string& proficiency,
                                      std::string& message) const
{
    static const std::unordered_set<std::string> validCategories = {
        "BACKEND", "FRONTEND", "DEVOPS", "QA", "OTHER"};
    static const std::unordered_set<std::string> validProficiency = {
        "BEGINNER", "INTERMEDIATE", "ADVANCED"};

    if (validCategories.find(category) == validCategories.end() ||
        validProficiency.find(proficiency) == validProficiency.end())
    {
        message = "Invalid skill category or proficiency.";
        return false;
    }
    return true;
}
