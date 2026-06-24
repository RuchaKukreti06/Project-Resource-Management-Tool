#include "manager/TimesheetsScreen.h"
#include "AuthSession.h"
#include "dto/ApiResponse.h"
#include "dto/TimesheetDTO.h"
#include <iomanip>
#include "api/ApiException.h"
#include "services/TimesheetClientService.h"

TimesheetsScreen::TimesheetsScreen(TimesheetClientService& tsService)
    : tsService_(tsService)
{
}

void TimesheetsScreen::displayMenu()
{
}

void TimesheetsScreen::show()
{
    while (true)
    {
        try
        {
            // clearScreen();
            decorator().render();
            
            std::cout << "Filter by week (DD-MM-YYYY) or press Enter for current week:\n";
            std::string weekInput = ScreenUtils::readLine("Week");

            // Validate format if provided — must be DD-MM-YYYY
            if (!weekInput.empty() &&
                !(weekInput.length() == 10 && weekInput[2] == '-' && weekInput[5] == '-'))
            {
                showError("Invalid date format. Use DD-MM-YYYY (e.g. 09-06-2026) or press Enter for current week.");
                ScreenUtils::readLine("Press Enter to try again");
                continue;
            }

            std::string formattedDate = weekInput;
            if (!weekInput.empty() && weekInput.length() == 10 && weekInput[2] == '-' && weekInput[5] == '-')
            {
                // Convert DD-MM-YYYY -> YYYY-MM-DD
                formattedDate = weekInput.substr(6, 4) + "-" + weekInput.substr(3, 2) + "-" + weekInput.substr(0, 2);
            }

            int managerId = api::AuthSession::instance().userId();
            auto response = tsService_.getManagerTimesheets(managerId, formattedDate);
            if (!response.success)
            {
                showError(response.message);
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }

            auto rows = response.data;
            // clearScreen();
            std::cout << "\n================ TIMESHEETS — MY TEAM ================\n";
            std::cout << std::left << std::setw(20) << "Employee"
                      << std::setw(20) << "Project"
                      << std::setw(8) << "Hrs"
                      << "Status\n";
            ScreenUtils::printDivider();

            for (const auto& row : rows)
            {
                std::cout << std::left << std::setw(20) << row.employeeName.substr(0, 19)
                          << std::setw(20) << row.projectName.substr(0, 19)
                          << std::setw(8) << row.hours
                          << row.status << "\n";
            }
            ScreenUtils::printDivider();

            std::cout << "[V] View employee timesheet detail     [B] Back\n";
            std::string choice = ScreenUtils::readLine("Enter choice");
            if (choice == "V" || choice == "v")
            {
                viewTimesheetDetail();
            }
            else if (choice == "B" || choice == "b")
            {
                break;
            }
        }
        catch (const ApiException& ex)
        {
            showError(ex.what());
            ScreenUtils::readLine("Press Enter to continue");
            break;
        }
        catch (const std::exception&)
        {
            showError("Something went wrong. Please try again.");
        }
    }
}

void TimesheetsScreen::handleInput()
{
}

void TimesheetsScreen::viewTimesheetDetail()
{
    try
    {
        std::string empId = ScreenUtils::readLine("Enter Employee ID");
        auto response = tsService_.getEmployeeTimesheets(std::stoi(empId));
        
        if (!response.success)
        {
            showError(response.message);
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto timesheets = response.data;
        // clearScreen();
        std::cout << "\n========== TIMESHEET HISTORY FOR EMPLOYEE ID: " << empId << " ==========\n";
        std::cout << std::left << std::setw(15) << "Week Start"
                  << "Status\n";
        ScreenUtils::printDivider();

        for (const auto& ts : timesheets)
        {
            std::cout << std::left << std::setw(15) << ts.weekStartDate
                      << ts.status << "\n";
        }
        ScreenUtils::printDivider();
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

ScreenDecorator TimesheetsScreen::decorator() const
{
    return ScreenDecorator("TIMESHEETS").withWidth(40).withPadding(2);
}
