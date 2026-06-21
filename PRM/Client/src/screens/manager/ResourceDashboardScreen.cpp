#include "manager/ResourceDashboardScreen.h"
#include "AuthSession.h"
#include "dto/ApiResponse.h"
#include "dto/EmployeeDTO.h"
#include "dto/ProjectDTO.h"
#include "dto/AllocationDTO.h"
#include "dto/TimesheetDTO.h"
#include <iomanip>
#include <set>
#include <sstream>

ResourceDashboardScreen::ResourceDashboardScreen()
{
}

void ResourceDashboardScreen::displayMenu()
{
    // Display is handled in show() dynamically
}

void ResourceDashboardScreen::show(ApiClient& apiClient)
{
    while (true)
    {
        try
        {
            int managerId = api::AuthSession::instance().userId();
            auto response = ApiListResponse<EmployeeDTO>::fromJson(apiClient.get("/managers/" + std::to_string(managerId) + "/team"));
            if (!response.success)
            {
                showError(response.message);
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }

            auto employees = response.data;
            // clearScreen();
            decorator().render();

            std::cout << "\nON BENCH\n";
            std::cout << std::left << std::setw(6) << "ID"
                      << std::setw(20) << "Name"
                      << std::setw(15) << "Department"
                      << "Skills\n";
            ScreenUtils::printDivider();

            int benchCount = 0;
            int activeCount = 0;

            std::vector<EmployeeDTO> activeList;

            for (const auto& emp : employees)
            {
                if (!emp.isActive) continue;

                int empId = emp.id;
                std::string status = emp.status;

                if (status == "BENCH")
                {
                    // Fetch skills
                    std::string skillsStr = "";
                    auto skillsRes = ApiListResponse<SkillDTO>::fromJson(apiClient.get("/employees/" + std::to_string(empId) + "/skills"));
                    if (skillsRes.success)
                    {
                        int count = 0;
                        for (const auto& s : skillsRes.data)
                        {
                            if (count > 0) skillsStr += ", ";
                            skillsStr += s.skillName;
                            count++;
                        }
                    }

                    std::cout << std::left << std::setw(6) << empId
                              << std::setw(20) << emp.fullName.substr(0, 19)
                              << std::setw(15) << emp.department.substr(0, 14)
                              << skillsStr << "\n";
                    benchCount++;
                }
                else
                {
                    activeList.push_back(emp);
                    activeCount++;
                }
            }

            std::cout << "\nACTIVE EMPLOYEES\n";
            std::cout << std::left << std::setw(6) << "ID"
                      << std::setw(20) << "Name"
                      << std::setw(15) << "Alloc %"
                      << "Availability\n";
            ScreenUtils::printDivider();

            for (const auto& emp : activeList)
            {
                int util = emp.totalUtilisation;
                std::string avail = (util == 0) ? "FULL" : (util >= 100 ? "0% free" : std::to_string(100 - util) + "% free");

                std::cout << std::left << std::setw(6) << emp.id
                          << std::setw(20) << emp.fullName.substr(0, 19)
                          << std::setw(15) << (std::to_string(util) + "%")
                          << avail << "\n";
            }
            ScreenUtils::printDivider();
            std::cout << "Bench: " << benchCount << "   |   Active: " << activeCount << "\n\n";

            std::cout << "[D] Drill into employee details     [B] Back\n";
            std::string choice = ScreenUtils::readLine("Enter choice");
            if (choice == "D" || choice == "d")
            {
                drillIntoEmployeeDetails(apiClient);
            }
            else if (choice == "B" || choice == "b")
            {
                break;
            }
        }
        catch (const std::exception& ex)
        {
            showError(std::string("Dashboard error: ") + ex.what());
            ScreenUtils::readLine("Press Enter to continue");
            break;
        }
    }
}

void ResourceDashboardScreen::handleInput(ApiClient& apiClient)
{
}

void ResourceDashboardScreen::drillIntoEmployeeDetails(ApiClient& apiClient)
{
    try
    {
        std::string empId = ScreenUtils::readLine("Enter Employee ID");
        int managerId = api::AuthSession::instance().userId();
        auto response = ApiListResponse<EmployeeDTO>::fromJson(apiClient.get("/managers/" + std::to_string(managerId) + "/team"));
        if (!response.success) return;

        EmployeeDTO targetEmp;
        bool found = false;
        for (const auto& emp : response.data)
        {
            if (std::to_string(emp.id) == empId)
            {
                targetEmp = emp;
                found = true;
                break;
            }
        }

        if (!found)
        {
            showError("Employee not found.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        // clearScreen();
        std::cout << "\n── " << targetEmp.fullName << " ─────────────────────────────────\n";
        std::cout << "Department     : " << targetEmp.department << "\n";
        std::cout << "Designation    : " << targetEmp.designation << "\n";
        std::cout << "Current Status : " << targetEmp.status << "\n";

        // Fetch skills
        std::string skillsStr = "";
        auto skillsRes = ApiListResponse<SkillDTO>::fromJson(apiClient.get("/employees/" + empId + "/skills"));
        if (skillsRes.success)
        {
            int count = 0;
            for (const auto& s : skillsRes.data)
            {
                if (count > 0) skillsStr += ", ";
                skillsStr += s.skillName + " (" + s.proficiency + ")";
                count++;
            }
        }
        std::cout << "Profile Skills : " << (skillsStr.empty() ? "None" : skillsStr) << "\n\n";

        // Fetch active allocations across all projects
        std::cout << "Active Allocations:\n";
        std::cout << std::left << std::setw(20) << "  Project" 
                  << std::setw(8) << "%" 
                  << std::setw(12) << "From" 
                  << std::setw(12) << "To" << "\n";
        ScreenUtils::printDivider();

        auto projResponse = ApiListResponse<ProjectDTO>::fromJson(apiClient.get("/projects"));
        if (projResponse.success)
        {
            for (const auto& proj : projResponse.data)
            {
                int projId = proj.id;
                auto allocsRes = ApiListResponse<AllocationDTO>::fromJson(apiClient.get("/projects/" + std::to_string(projId) + "/allocations"));
                if (!allocsRes.success) continue;
                for (const auto& alloc : allocsRes.data)
                {
                    if (std::to_string(alloc.employeeId) == empId)
                    {
                        std::cout << "  " << std::left << std::setw(18) << proj.name.substr(0, 17)
                                  << std::setw(8) << (std::to_string(alloc.utilizationPercentage) + "%")
                                  << std::setw(12) << alloc.fromDate
                                  << std::setw(12) << alloc.toDate << "\n";
                    }
                }
            }
        }
        std::cout << "\n";

        // Fetch recent activity tags from timesheets (real data)
        std::cout << "Recent Activity Tags (last 4 weeks):\n  ";
        auto tsRes = ApiListResponse<TimesheetDTO>::fromJson(apiClient.get("/employees/" + empId + "/timesheets"));
        std::set<std::string> tagSet;
        if (tsRes.success)
        {
            int weekCount = 0;
            for (const auto& ts : tsRes.data)
            {
                if (weekCount++ >= 4) break;
                auto detailRes = ApiListResponse<TimesheetEntryDTO>::fromJson(apiClient.get("/timesheets/" + std::to_string(ts.id)));
                if (detailRes.success)
                {
                    for (const auto& row : detailRes.data)
                    {
                        std::string tags = row.tags;
                        std::istringstream ss(tags);
                        std::string tag;
                        while (std::getline(ss, tag, ','))
                        {
                            auto start = tag.find_first_not_of(" ");
                            if (start != std::string::npos)
                                tagSet.insert(tag.substr(start));
                        }
                    }
                }
            }
        }
        std::string tagsStr;
        for (const auto& t : tagSet) { if (!tagsStr.empty()) tagsStr += ", "; tagsStr += t; }
        std::cout << (tagsStr.empty() ? "No recent activity recorded." : tagsStr) << "\n\n";

        ScreenUtils::readLine("Press Enter to go back");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error drilling into employee details: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator ResourceDashboardScreen::decorator() const
{
    return ScreenDecorator("RESOURCE DASHBOARD").withWidth(40).withPadding(2);
}
