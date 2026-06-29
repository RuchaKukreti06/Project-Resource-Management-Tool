#ifndef ALL_ALLOCATIONS_SCREEN_H
#define ALL_ALLOCATIONS_SCREEN_H

#include "screens/Screen.h"
#include "dto/ProjectDTO.h"
#include "dto/AllocationDTO.h"
#include <map>
#include <vector>
#include <string>
#include <optional>
#include "admin/adminConstants.h"

class ProjectClientService;
class AllocationClientService;
class EmployeeClientService;

class AllAllocationsScreen : public Screen
{
   private:
    ProjectClientService& projService_;
    AllocationClientService& allocService_;
    EmployeeClientService& empService_;

    struct AllocationRow {
        std::string employeeName;
        std::string projectName;
        std::string utilization;
        std::string fromDate;
        std::string toDate;
    };

    std::optional<std::vector<AllocationRow>> fetchAllocationRows();
    std::map<int, std::string> buildEmployeeNameMap();
    bool appendProjectAllocations(
        const ProjectDTO& project, 
        const std::map<int, std::string>& employeeNameById, 
        std::vector<AllocationRow>& rows);
    
    void displayAllocations(const std::vector<AllocationRow>& rows);

   public:
    AllAllocationsScreen(ProjectClientService& projService, 
                         AllocationClientService& allocService, 
                         EmployeeClientService& empService);

    void show() override;
    void displayMenu() override;
    void handleInput() override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif // ALL_ALLOCATIONS_SCREEN_H
