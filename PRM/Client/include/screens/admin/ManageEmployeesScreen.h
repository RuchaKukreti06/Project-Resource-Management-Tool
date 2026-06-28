#ifndef MANAGE_EMPLOYEES_SCREEN_H
#define MANAGE_EMPLOYEES_SCREEN_H

#include "screens/Screen.h"
#include "dto/EmployeeDTO.h"
#include <vector>
#include <string>
#include <optional>
#include "admin/adminConstants.h"

class EmployeeClientService;

class ManageEmployeesScreen : public Screen
{
   public:
    ManageEmployeesScreen(EmployeeClientService& empService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    void displayEmployees(const std::vector<EmployeeDTO>& employees, const std::string& filterStatus, const std::string& filterDept);
    EmployeeClientService& empService_;
    bool keepRunning_ = true;
    
    // Original methods
    void viewAllEmployees();
    void updateEmployee();
    void deactivateEmployee();
    void manageEmployeeSkills();
    void assignManager();

    // New Common Helpers
    std::optional<int> promptForId(const std::string& prompt);
    std::optional<EmployeeDTO> fetchEmployeeById(int empId);
    void handleStandardExceptions(const std::exception& ex);

    // New viewAllEmployees Helpers
    void displayFilterMenu();
    std::optional<std::string> promptForStatusFilter();
    std::string promptForDepartmentFilter();
    void applyFilterChoice(const std::string& filterChoice, std::string& filterStatus, std::string& filterDept);

    std::vector<EmployeeDTO> filterEmployees(const std::vector<EmployeeDTO>& employees, const std::string& filterStatus, const std::string& filterDept);
    void displayEmployeeTable(const std::vector<EmployeeDTO>& employees);
    void displayEmployeeSummary(const std::vector<EmployeeDTO>& employees);
    bool promptFilterOptions(std::string& filterStatus, std::string& filterDept);

    // New updateEmployee Helpers
    void promptForBasicDetails(const EmployeeDTO& targetEmp, UpdateEmployeeRequest& req);
    void promptForEmployeeStatus(const EmployeeDTO& targetEmp, UpdateEmployeeRequest& req);
    UpdateEmployeeRequest promptForEmployeeUpdateRequest(const EmployeeDTO& targetEmp);

    // New manageEmployeeSkills Helpers
    void displayCurrentSkills(const std::vector<SkillDTO>& skills);
    std::string promptForSkillCategory();
    std::string promptForProficiency();
    AddSkillRequest promptForAddSkillRequest();
    std::optional<int> promptForSkillIdFromList(const std::vector<SkillDTO>& skills, const std::string& prompt);
    UpdateSkillRequest promptForUpdateSkillRequest();
    
    void executeAddSkill(int empId);
    void executeUpdateSkill(int empId, const std::vector<SkillDTO>& skills);
    void executeRemoveSkill(int empId, const std::vector<SkillDTO>& skills);
    bool handleSkillOption(const std::string& selectedOption, int empId, const std::vector<SkillDTO>& skills);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
