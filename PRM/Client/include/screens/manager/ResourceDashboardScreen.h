#ifndef RESOURCE_DASHBOARD_SCREEN_H
#define RESOURCE_DASHBOARD_SCREEN_H

#include "screens/Screen.h"
#include "manager/managerConstants.h"
#include <optional>
#include <vector>
#include <string>
#include "dto/EmployeeDTO.h"

class EmployeeClientService;
class ProjectClientService;
class AllocationClientService;
class TimesheetClientService;

class ResourceDashboardScreen : public Screen
{
   public:
    ResourceDashboardScreen(EmployeeClientService& empService, ProjectClientService& projService, AllocationClientService& allocService, TimesheetClientService& tsService, int currentUserId);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    struct BenchEmployeeData {
        int id;
        std::string name;
        std::string department;
        std::string skills;
    };
    
    struct ActiveEmployeeData {
        int id;
        std::string name;
        std::string allocPercent;
        std::string availability;
    };

    struct AllocationDisplayData {
        std::string projectName;
        std::string utilPercent;
        std::string fromDate;
        std::string toDate;
    };

    void displayBenchEmployees(const std::vector<BenchEmployeeData>& benchEmployees);
    void displayActiveEmployees(const std::vector<ActiveEmployeeData>& activeEmployees);
    void displayAllocations(const std::vector<AllocationDisplayData>& allocations);

    EmployeeClientService& empService_;
    ProjectClientService& projService_;
    AllocationClientService& allocService_;
    TimesheetClientService& tsService_;
    int currentUserId_;
    void viewDashboard();
    void drillIntoEmployeeDetails();
    
    std::optional<int> promptForEmployeeId();
    void fetchEmployeeProfile(int employeeId);
    void displayEmployeeAllocations(int employeeId);
    void displayRecentActivity(int employeeId);
    std::vector<EmployeeDTO> categorizeEmployees(std::vector<BenchEmployeeData>& benchData, std::vector<ActiveEmployeeData>& activeData, int& benchCount, int& activeCount);
    void printDashboardSummary(int benchCount, int activeCount);
    EmployeeDTO fetchEmployeeDTO(int employeeId);
    void printEmployeeProfile(const EmployeeDTO& targetEmployee);
    std::string buildSkillSummary(int employeeId, bool includeProficiency);

    bool keepRunning_ = true;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
