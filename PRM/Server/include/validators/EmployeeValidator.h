#pragma once

#include <string>

class EmployeeValidator
{
public:
    // Validates that employee.id is positive.
    bool validateId(int employeeId, std::string& message) const;

    // Validates that category and proficiency are from the allowed enum sets.
    bool validateSkill(const std::string& category,
                       const std::string& proficiency,
                       std::string& message) const;
};
