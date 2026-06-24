#include "employee/EmployeeScreen.h"
#include "employee/SubmitTimesheetScreen.h"
#include "AuthSession.h"
#include "dto/ApiResponse.h"
#include "dto/EmployeeDTO.h"
#include "dto/TimesheetDTO.h"
#include "dto/ProjectDTO.h"
#include "dto/AllocationDTO.h"
#include "AuthSession.h"
#include <iomanip>
#include <map>
#include "api/ApiException.h"
#include "services/EmployeeClientService.h"
#include "services/TimesheetClientService.h"
#include "services/ProjectClientService.h"
#include "services/AllocationClientService.h"

#include "app/Router.h"
#include "api/ISessionStore.h"

EmployeeScreen::EmployeeScreen(Router& router, api::ISessionStore& sessionStore, EmployeeClientService& empService, TimesheetClientService& tsService, ProjectClientService& projService, AllocationClientService& allocService)
    : router_(router), sessionStore_(sessionStore), empService_(empService), tsService_(tsService), projService_(projService), allocService_(allocService)
{
}

void EmployeeScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    
    if (hasMissingTimesheet_)
    {
        std::cout << "  \x1B[33m\xE2\x9A\xA0  Reminder: Timesheet for week " << missingWeekStr_ << " has not been submitted.\x1B[0m\n";
    }
    ScreenUtils::printDivider();

    std::cout << "1. Submit Timesheet\n";
    std::cout << "2. View My Timesheets\n";
    std::cout << "3. View My Allocations\n";
    std::cout << "4. Logout\n";
}

void EmployeeScreen::show()
{
    // Fetch Employee ID
    int userId = sessionStore_.userId();
    int empId = 0;
    auto empRes = empService_.viewAllEmployees();
    if (empRes.success)
    {
        for (const auto& e : empRes.data)
        {
            if (e.userId == userId)
            {
                empId = e.id;
                break;
            }
        }
    }

    while (true)
    {
        // Calculate last completed week's Monday
        std::time_t now = std::time(nullptr);
        std::tm local = {};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        local = *std::localtime(&now);
#endif
        int daysSinceMonday = local.tm_wday == 0 ? 6 : local.tm_wday - 1;
        std::time_t lastMondayTime = now - ((daysSinceMonday + 7) * 24 * 60 * 60);
        std::tm lastMonday = {};
#ifdef _WIN32
        localtime_s(&lastMonday, &lastMondayTime);
#else
        lastMonday = *std::localtime(&lastMondayTime);
#endif
        char bufferIso[11] = {0};
        std::strftime(bufferIso, sizeof(bufferIso), "%Y-%m-%d", &lastMonday);
        std::string lastMondayIso = bufferIso;
        char bufferDisp[15] = {0};
        std::strftime(bufferDisp, sizeof(bufferDisp), "%d-%b-%Y", &lastMonday);
        missingWeekStr_ = bufferDisp;

        hasMissingTimesheet_ = true;
        if (empId > 0)
        {
            auto tsRes = tsService_.getEmployeeTimesheets(empId);
            if (tsRes.success)
            {
                for (const auto& ts : tsRes.data)
                {
                    if (ts.weekStartDate == lastMondayIso && ts.status == "SUBMITTED")
                    {
                        hasMissingTimesheet_ = false;
                        break;
                    }
                }
            }
        }

        displayMenu();
        handleInput();
        if (!sessionStore_.isLoggedIn())
        {
            break;
        }
    }
}

void EmployeeScreen::handleInput()
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        router_.navigateToSubmitTimesheet();
    }
    else if (choice == "2")
    {
        viewMyTimesheets();
    }
    else if (choice == "3")
    {
        viewMyAllocations();
    }
    else if (choice == "4")
    {
        sessionStore_.logout();
        showSuccess("Logged out successfully.");
    }
    else
    {
        showError("Invalid option. Please enter 1–4.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void EmployeeScreen::viewMyTimesheets()
{
    try
    {
        // Find Employee ID
        int userId = sessionStore_.userId();
        int empId = 0;
        auto empRes = empService_.viewAllEmployees();
        if (empRes.success)
        {
            for (const auto& e : empRes.data)
            {
                if (e.userId == userId)
                {
                    empId = e.id;
                    break;
                }
            }
        }

        if (empId == 0)
        {
            showError("No employee profile linked to your user account.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto response = tsService_.getEmployeeTimesheets(empId);
        if (!response.success)
        {
            showError(response.message);
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto timesheets = response.data;
        // clearScreen();
        std::cout << "\n================ MY TIMESHEETS ================\n";
        std::cout << std::left << std::setw(15) << "Week Start"
                  << std::setw(15) << "Total Hrs"
                  << "Status\n";
        ScreenUtils::printDivider();

        std::vector<TimesheetDTO> tsList;
        for (const auto& ts : timesheets)
        {
            tsList.push_back(ts);
            
            std::string dateIso = ts.weekStartDate;
            int y, m, d;
            std::string dispDate = dateIso;
            if (std::sscanf(dateIso.c_str(), "%d-%d-%d", &y, &m, &d) == 3)
            {
                std::tm tm = {};
                tm.tm_year = y - 1900;
                tm.tm_mon = m - 1;
                tm.tm_mday = d;
                char b[15] = {0};
                std::strftime(b, sizeof(b), "%d-%b-%Y", &tm);
                dispDate = b;
            }

            std::cout << std::left << std::setw(15) << dispDate
                      << std::setw(15) << (std::to_string(ts.totalHours) + " hrs")
                      << ts.status << "\n";
        }
        ScreenUtils::printDivider();
        
        while (true)
        {
            std::cout << "\n[V] View week details     [B] Back\n";
            std::string choice = ScreenUtils::readLine("Enter option");
            if (choice == "V" || choice == "v")
            {
                std::string dateToView = ScreenUtils::readLine("Enter Week Start Date (YYYY-MM-DD)");

                // Validate format before searching
                int yy, mm, dd;
                if (dateToView.length() != 10 ||
                    std::sscanf(dateToView.c_str(), "%d-%d-%d", &yy, &mm, &dd) != 3 ||
                    mm < 1 || mm > 12 || dd < 1 || dd > 31)
                {
                    showError("Invalid date format. Use YYYY-MM-DD (e.g. 2026-06-09).");
                    continue;
                }

                int selectedId = 0;
                std::string st = "";
                for (const auto& ts : tsList)
                {
                    if (ts.weekStartDate == dateToView)
                    {
                        selectedId = ts.id;
                        st = ts.status;
                        break;
                    }
                }

                if (selectedId > 0)
                {
                    viewTimesheetDetails(selectedId, dateToView, st);
                    // Stay in [V]/[B] loop after viewing details
                }
                else
                {
                    showError("No timesheet found for that date.");
                }
            }
            else if (choice == "B" || choice == "b")
            {
                break;
            }
        }
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void EmployeeScreen::viewMyAllocations()
{
    try
    {
        // Find Employee ID
        int userId = sessionStore_.userId();
        int empId = 0;
        auto empRes = empService_.viewAllEmployees();
        if (empRes.success)
        {
            for (const auto& e : empRes.data)
            {
                if (e.userId == userId)
                {
                    empId = e.id;
                    break;
                }
            }
        }

        if (empId == 0)
        {
            showError("No employee profile linked to your user account.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        // clearScreen();
        std::cout << "\n================ MY ALLOCATIONS ================\n";
        std::cout << std::left << std::setw(20) << "Project"
                  << std::setw(8) << "%"
                  << std::setw(12) << "From"
                  << std::setw(12) << "To"
                  << "Status\n";
        ScreenUtils::printDivider();

        auto projRes = projService_.viewAllProjects();
        std::map<int, std::string> projectNames;
        if (projRes.success)
        {
            for (const auto& proj : projRes.data)
            {
                projectNames[proj.id] = proj.name;
            }
        }

        int count = 0;
        int totalUtil = 0;
        auto allocs = allocService_.getEmployeeAllocations(empId);
        if (allocs.success)
        {
            for (const auto& alloc : allocs.data)
            {
                std::string pName = projectNames.count(alloc.projectId) ? projectNames[alloc.projectId] : "Unknown Project";
                std::cout << std::left << std::setw(20) << pName.substr(0, 19)
                          << std::setw(8) << (std::to_string(alloc.utilizationPercentage) + "%")
                          << std::setw(12) << alloc.fromDate
                          << std::setw(12) << alloc.toDate
                          << "ACTIVE\n";
                totalUtil += alloc.utilizationPercentage;
                count++;
            }
        }
        ScreenUtils::printDivider();
        std::cout << "Total Utilisation: " << totalUtil << "%\n\n";

        if (count == 0)
        {
            showInfo("You have no active allocations.");
        }
        ScreenUtils::readLine("Press Enter to go back");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void EmployeeScreen::viewTimesheetDetails(int timesheetId, const std::string& weekStart, const std::string& status)
{
    auto response = tsService_.getTimesheetEntries(timesheetId);
    if (!response.success)
    {
        showError(response.message);
        ScreenUtils::readLine("Press Enter to continue");
        return;
    }

    std::string dispDate = weekStart;
    int y, m, d;
    if (std::sscanf(weekStart.c_str(), "%d-%d-%d", &y, &m, &d) == 3)
    {
        std::tm tm = {};
        tm.tm_year = y - 1900;
        tm.tm_mon = m - 1;
        tm.tm_mday = d;
        char b[15] = {0};
        std::strftime(b, sizeof(b), "%d-%b-%Y", &tm);
        dispDate = b;
    }

    std::cout << "\n\n── Week: " << dispDate << " — Status: " << status << " ─────\n\n";
    std::cout << std::left << std::setw(20) << "Project"
              << std::setw(10) << "Hrs"
              << "Activity Tags\n";
    ScreenUtils::printDivider();

    int total = 0;
    for (const auto& row : response.data)
    {
        int hrs = row.hoursWorked;
        total += hrs;
        std::cout << std::left << std::setw(20) << row.projectName.substr(0, 19)
                  << std::setw(10) << hrs
                  << row.tags << "\n";
    }
    ScreenUtils::printDivider();
    std::cout << "Total: " << total << " hrs\n\n";
    ScreenUtils::readLine("Press Enter to go back");
}

ScreenDecorator EmployeeScreen::decorator() const
{
    std::string username = sessionStore_.username();
    return ScreenDecorator("Welcome, " + username + "!").withWidth(40).withPadding(2);
}
