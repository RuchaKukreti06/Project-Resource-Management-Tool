#include "admin/AdminScreen.h"
#include "admin/ManageEmployeesScreen.h"
#include "admin/ManageProjectsScreen.h"
#include "admin/ManageUsersScreen.h"
#include "admin/SystemConfigScreen.h"
#include "AuthSession.h"
#include "dto/ApiResponse.h"
#include "dto/ProjectDTO.h"
#include "dto/EmployeeDTO.h"
#include <map>
#include "api/ApiException.h"
#include "services/ProjectClientService.h"
#include "services/AllocationClientService.h"
#include "services/EmployeeClientService.h"
#include "app/Router.h"

AdminScreen::AdminScreen(Router& router, api::ISessionStore& sessionStore, ProjectClientService& projService, AllocationClientService& allocService, EmployeeClientService& empService)
    : router_(router), sessionStore_(sessionStore), projService_(projService), allocService_(allocService), empService_(empService)
{
}

void AdminScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    std::cout << "1. Manage Employees\n";
    std::cout << "2. Manage Projects\n";
    std::cout << "3. View All Allocations\n";
    std::cout << "4. Manage Users\n";
    std::cout << "5. System Configuration\n";
    std::cout << "6. Logout\n";
}

void AdminScreen::show()
{
    while (true)
    {
        displayMenu();
        handleInput();
        if (!sessionStore_.isLoggedIn())
        {
            break;
        }
    }
}

void AdminScreen::handleInput()
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        router_.navigateToManageEmployees();
    }
    else if (choice == "2")
    {
        router_.navigateToManageProjects();
    }
    else if (choice == "3")
    {
        // View All Allocations
        try
        {
            auto response = projService_.viewAllProjects();
            if (!response.success)
            {
                showError(response.message);
                return;
            }
            auto projects = response.data;
            // clearScreen();
            std::cout << "\n================ ALL ACTIVE ALLOCATIONS ================\n";
            std::cout << std::left << std::setw(15) << "Employee" 
                      << std::setw(20) << "Project" 
                      << std::setw(8)  << "%" 
                      << std::setw(12) << "From" 
                      << std::setw(12) << "To" << "\n";
            ScreenUtils::printDivider();

            int totalAllocations = 0;
            
            auto empRes = empService_.viewAllEmployees();
            std::map<int, std::string> empMap;
            if (empRes.success) {
                for (const auto& e : empRes.data) {
                    empMap[e.id] = e.fullName;
                }
            }

            for (const auto& proj : projects)
            {
                auto allocRes = allocService_.getProjectAllocations(proj.id);
                if (!allocRes.success) continue;
                for (const auto& alloc : allocRes.data)
                {
                    std::string empName = "Emp " + std::to_string(alloc.employeeId);
                    if (empMap.find(alloc.employeeId) != empMap.end()) {
                        empName = empMap[alloc.employeeId];
                    }

                    std::cout << std::left << std::setw(15) << empName.substr(0, 14)
                              << std::setw(20) << proj.name.substr(0, 19)
                              << std::setw(8)  << (std::to_string(alloc.utilizationPercentage) + "%")
                              << std::setw(12) << alloc.fromDate
                              << std::setw(12) << alloc.toDate << "\n";
                    totalAllocations++;
                }
            }
            ScreenUtils::printDivider();
            std::cout << "Total Active Allocations: " << totalAllocations << "\n\n";
            ScreenUtils::readLine("Press Enter to go back");
        }
        catch (const ApiException& e)
        {
            showError(e.what());
            ScreenUtils::readLine("Press Enter to continue");
        }
        catch (const std::exception&)
        {
            showError("Something went wrong. Please try again.");
        }
    }
    else if (choice == "4")
    {
        router_.navigateToManageUsers();
    }
    else if (choice == "5")
    {
        router_.navigateToSystemConfig();
    }
    else if (choice == "6")
    {
        sessionStore_.logout();
        showSuccess("Logged out successfully.");
    }
    else
    {
        showError("Invalid option. Please enter 1–6.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator AdminScreen::decorator() const
{
    return ScreenDecorator("ADMIN PANEL").withWidth(40).withPadding(2);
}
