#include "employee/EmployeeScreen.h"
#include "utils/ConsoleInput.h"
#include "employee/SubmitTimesheetScreen.h"
#include "AuthSession.h"
#include "dto/ApiResponse.h"
#include "dto/EmployeeDTO.h"
#include "dto/TimesheetDTO.h"
#include "dto/ProjectDTO.h"
#include "dto/AllocationDTO.h"
#include <iomanip>
#include <map>
#include "api/ApiException.h"
#include "services/EmployeeClientService.h"
#include "services/TimesheetClientService.h"
#include "services/ProjectClientService.h"
#include "services/AllocationClientService.h"
#include "screens/ScreenUtils.h"
#include "utils/DateUtils.h"

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
    int empId = 0;
    auto empRes = empService_.getMe();
    if (empRes.success && empRes.data)
    {
        empId = empRes.data->id;
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

        hasMissingTimesheet_ = false;
        if (empId > 0)
        {
            auto allocRes = allocService_.getEmployeeAllocations(empId);
            bool hasAllocations = false;
            if (allocRes.success)
            {
                for (const auto& alloc : allocRes.data)
                {
                    if (alloc.fromDate <= lastMondayIso && (alloc.toDate.empty() || alloc.toDate >= lastMondayIso))
                    {
                        hasAllocations = true;
                        break;
                    }
                }
            }

            if (hasAllocations)
            {
                hasMissingTimesheet_ = true;
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
    std::string choice = ConsoleInput::readLine("Enter option");
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
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

void EmployeeScreen::viewMyTimesheets()
{
    try
    {
        int empId = 0;
        auto empRes = empService_.getMe();
        if (empRes.success && empRes.data)
        {
            empId = empRes.data->id;
        }

        if (empId == 0)
        {
            showError("No employee profile linked to your user account.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        auto response = tsService_.getEmployeeTimesheets(empId);
        if (!response.success)
        {
            showError(response.message);
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        auto timesheets = response.data;
        
        std::vector<TimesheetDisplayData> displayData;

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

            displayData.push_back({
                dispDate,
                std::to_string(ts.totalHours) + " hrs",
                ts.status
            });
        }
        
        displayMyTimesheets(displayData);
        
        while (true)
        {
            std::cout << "\n[V] View week details     [B] Back\n";
            std::string choice = ConsoleInput::readLine("Enter option");
            if (choice == "V" || choice == "v")
            {
                std::string dateToView = ConsoleInput::readLine("Enter Week Start Date (YYYY-MM-DD)");
                if (dateToView.empty())
                {
                    continue;
                }

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
        ConsoleInput::waitForEnter("Press Enter to continue\n");
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
        int empId = 0;
        auto empRes = empService_.getMe();
        if (empRes.success && empRes.data)
        {
            empId = empRes.data->id;
        }

        if (empId == 0)
        {
            showError("No employee profile linked to your user account.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        std::vector<AllocationDisplayData> allocData;

        int count = 0;
        int totalUtil = 0;
        auto allocs = allocService_.getEmployeeAllocations(empId);
        if (allocs.success)
        {
            for (const auto& alloc : allocs.data)
            {
                std::string pName = alloc.projectName.empty() ? ("Project " + std::to_string(alloc.projectId)) : alloc.projectName;
                
                std::string status = "ACTIVE";
                std::string today = DateUtils::getCurrentDateYYYYMMDD();
                
                if (!alloc.toDate.empty() && alloc.toDate < today) {
                    status = "INACTIVE";
                } else if (!alloc.fromDate.empty() && alloc.fromDate > today) {
                    status = "UPCOMING";
                }
                
                allocData.push_back({
                    pName.substr(0, 19),
                    std::to_string(alloc.utilizationPercentage) + "%",
                    ScreenUtils::valueOrDash(alloc.fromDate),
                    ScreenUtils::valueOrDash(alloc.toDate),
                    status
                });
                
                if (status == "ACTIVE") {
                    totalUtil += alloc.utilizationPercentage;
                }
            }
        }
        
        displayMyAllocations(allocData, totalUtil);

        if (allocData.empty())
        {
            showInfo("You have no active allocations.");
        }
        ConsoleInput::waitForEnter("Press Enter to go back\n");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
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
        ConsoleInput::waitForEnter("Press Enter to continue\n");
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

    int total = 0;
    for (const auto& row : response.data)
    {
        total += row.hoursWorked;
    }

    std::string title = "Week: " + dispDate + " — Status: " + status;
    displayTimesheetDetailsList(title, response.data, total);
    ConsoleInput::waitForEnter("Press Enter to go back\n");
}

ScreenDecorator EmployeeScreen::decorator() const
{
    std::string username = sessionStore_.username();
    return ScreenDecorator("Welcome, " + username + "!").withWidth(40).withPadding(2);
}

void EmployeeScreen::displayMyTimesheets(const std::vector<TimesheetDisplayData>& timesheets)
{
    std::cout << "\n================ MY TIMESHEETS ================\n";
    std::cout << std::left
              << std::setw(15) << "Week Start"
              << std::setw(15) << "Total Hrs"
              << std::setw(15) << "Status" << "\n";
    std::cout << "─────────────────────────────────────────────\n";

    for (const auto& ts : timesheets)
    {
        std::cout << std::left
                  << std::setw(15) << ScreenUtils::valueOrDash(ts.dispDate)
                  << std::setw(15) << ScreenUtils::valueOrDash(ts.totalHrs)
                  << std::setw(15) << ScreenUtils::valueOrDash(ts.status) << "\n";
    }
    std::cout << "─────────────────────────────────────────────\n";
}

void EmployeeScreen::displayMyAllocations(const std::vector<AllocationDisplayData>& allocations, int totalUtil)
{
    std::cout << "\n================ MY ALLOCATIONS ================\n";
    std::cout << std::left
              << std::setw(20) << "Project"
              << std::setw(8) << "%"
              << std::setw(12) << "From"
              << std::setw(12) << "To"
              << std::setw(12) << "Status" << "\n";
    std::cout << "────────────────────────────────────────────────────────────────\n";

    for (const auto& alloc : allocations)
    {
        std::cout << std::left
                  << std::setw(20) << ScreenUtils::truncate(ScreenUtils::valueOrDash(alloc.projectName), 19)
                  << std::setw(8) << ScreenUtils::valueOrDash(alloc.utilPercent)
                  << std::setw(12) << ScreenUtils::valueOrDash(alloc.fromDate)
                  << std::setw(12) << ScreenUtils::valueOrDash(alloc.toDate)
                  << std::setw(12) << ScreenUtils::valueOrDash(alloc.status) << "\n";
    }
    std::cout << "────────────────────────────────────────────────────────────────\n";
    std::cout << "Total Utilisation: " << totalUtil << "%\n\n";
}

void EmployeeScreen::displayTimesheetDetailsList(const std::string& title, const std::vector<TimesheetEntryDTO>& entries, int totalHrs)
{
    std::cout << "\n================ " << title << " ================\n";
    std::cout << std::left
              << std::setw(20) << "Project"
              << std::setw(10) << "Hrs"
              << std::setw(30) << "Activity Tags" << "\n";
    std::cout << "────────────────────────────────────────────────────────────\n";

    for (const auto& row : entries)
    {
        std::cout << std::left
                  << std::setw(20) << ScreenUtils::truncate(ScreenUtils::valueOrDash(row.projectName), 19)
                  << std::setw(10) << row.hoursWorked
                  << std::setw(30) << ScreenUtils::truncate(ScreenUtils::valueOrDash(row.tags), 29) << "\n";
    }
    std::cout << "────────────────────────────────────────────────────────────\n";
    std::cout << "Total: " << totalHrs << " hrs\n\n";
}
