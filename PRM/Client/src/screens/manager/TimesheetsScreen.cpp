#include "manager/TimesheetsScreen.h"

#include <iomanip>

#include "api/ApiException.h"
#include "dto/ApiResponse.h"
#include "dto/TimesheetDTO.h"
#include "screens/ScreenUtils.h"
#include "services/TimesheetClientService.h"
#include "services/EmployeeClientService.h"
#include "dto/EmployeeDTO.h"
#include "utils/ConsoleInput.h"
#include "utils/DateUtils.h"

using namespace ManagerConstants;
using namespace ManagerConstants::Timesheets;

TimesheetsScreen::TimesheetsScreen(TimesheetClientService& tsService, EmployeeClientService& empService, int currentUserId)
    : tsService_(tsService), empService_(empService), currentUserId_(currentUserId)
{
}

void TimesheetsScreen::displayMenu()
{
}

void TimesheetsScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        viewTimesheetsBoard();
        if (keepRunning_)
        {
            handleInput();
        }
    }
}

void TimesheetsScreen::viewTimesheetsBoard()
{
    try
    {
        decorator().render();
        auto dateOpt = promptWeekFilter();
        if (!dateOpt) return;

        fetchManagerTimesheets(dateOpt.value());
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
        keepRunning_ = false;
    }
    catch (const std::exception&)
    {
        showError(Messages::SOMETHING_WENT_WRONG);
        keepRunning_ = false;
    }
}

std::optional<std::string> TimesheetsScreen::promptWeekFilter()
{
    while (true)
    {
        std::cout << "Filter by week (YYYY-MM-DD) or press Enter for current week:\n";
        std::string weekInput = ConsoleInput::readLine("Week");

        if (weekInput.empty())
        {
            return "";
        }

        std::string errorMsg = DateUtils::validateDateYYYYMMDD(weekInput, true);
        if (!errorMsg.empty())
        {
            showError(errorMsg);
            ConsoleInput::waitForEnter("Press Enter to try again\n");
            continue;
        }

        return weekInput;
    }
}

void TimesheetsScreen::fetchManagerTimesheets(const std::string& formattedDate)
{
    int managerId = currentUserId_;
    auto response = tsService_.getManagerTimesheets(managerId, formattedDate);
    if (!response.success)
    {
        showError(response.message);
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
        keepRunning_ = false;
        return;
    }

    auto rows = response.data;
    displayManagerTimesheets(rows);
}

void TimesheetsScreen::handleInput()
{
    std::cout << "[" << OPT_VIEW_DETAIL << "] View employee timesheet detail     [" << OPT_BACK
              << "] Back\n";
    std::string choice = ConsoleInput::readLine("Enter choice");
    if (ScreenUtils::equalsIgnoreCase(choice, OPT_VIEW_DETAIL))
    {
        viewTimesheetDetail();
    }
    else if (ScreenUtils::equalsIgnoreCase(choice, OPT_BACK))
    {
        keepRunning_ = false;
    }
    else
    {
        showError(Messages::INVALID_OPTION);
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
    }
}

void TimesheetsScreen::viewTimesheetDetail()
{
    try
    {
        auto empIdOpt = promptForEmployeeId();
        if (!empIdOpt) return;
        int employeeId = empIdOpt.value();

        auto teamResponse = empService_.getTeamEmployees(currentUserId_);
        if (!teamResponse.success)
        {
            showError("Failed to fetch your team: " + teamResponse.message);
            ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
            return;
        }

        bool found = false;
        for (const auto& emp : teamResponse.data)
        {
            if (emp.id == employeeId)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            showError("You can only view timesheets for employees in your team.");
            ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
            return;
        }

        fetchEmployeeTimesheets(employeeId);
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_GO_BACK);
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
    }
    catch (const std::exception&)
    {
        showError(Messages::SOMETHING_WENT_WRONG);
    }
}

std::optional<int> TimesheetsScreen::promptForEmployeeId()
{
    std::string employeeIdInput = ConsoleInput::readLine("Enter Employee ID");
    if (employeeIdInput.empty()) return std::nullopt;
    auto parsedEmployeeId = ScreenUtils::safeParseInt(employeeIdInput);

    if (!parsedEmployeeId)
    {
        showError(Messages::INVALID_SELECTION_FORMAT);
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
        return std::nullopt;
    }

    return parsedEmployeeId.value();
}

void TimesheetsScreen::fetchEmployeeTimesheets(int employeeId)
{
    auto response = tsService_.getEmployeeTimesheets(employeeId);

    if (!response.success)
    {
        throw ApiException(response.message);
    }

    auto timesheets = response.data;
    displayEmployeeTimesheets(employeeId, timesheets);
}

ScreenDecorator TimesheetsScreen::decorator() const
{
    return ScreenDecorator("TIMESHEETS")
        .withWidth(ManagerConstants::DEFAULT_PANEL_WIDTH)
        .withPadding(ManagerConstants::DEFAULT_PADDING);
}

void TimesheetsScreen::displayManagerTimesheets(const std::vector<ManagerTimesheetDTO>& rows)
{
    std::cout << "\n================ TIMESHEETS — MY TEAM ================\n";
    std::cout << std::left << std::setw(COL_EMPLOYEE) << "Employee" << std::setw(COL_PROJECT) << "Project"
              << std::setw(COL_HOURS) << "Hrs" << std::setw(COL_STATUS) << "Status" << "\n";
    std::cout << "────────────────────────────────────────────────────────\n";

    for (const auto& row : rows)
    {
        std::cout << std::left << std::setw(COL_EMPLOYEE)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(row.employeeName), COL_EMPLOYEE - 1)
                  << std::setw(COL_PROJECT)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(row.projectName), COL_PROJECT - 1)
                  << std::setw(COL_HOURS) << row.hours << std::setw(COL_STATUS)
                  << ScreenUtils::valueOrDash(row.status) << "\n";
    }
    std::cout << "────────────────────────────────────────────────────────\n";
}

void TimesheetsScreen::displayEmployeeTimesheets(int employeeId,
                                                 const std::vector<TimesheetDTO>& timesheets)
{
    std::cout << "\n================ TIMESHEET HISTORY FOR EMPLOYEE ID: " << employeeId
              << " ================\n";
    std::cout << std::left << std::setw(COL_WEEK_START) << "Week Start" << std::setw(COL_STATUS) << "Status" << "\n";
    std::cout << "──────────────────────────────────────────────\n";

    for (const auto& ts : timesheets)
    {
        std::cout << std::left << std::setw(COL_WEEK_START) << ScreenUtils::valueOrDash(ts.weekStartDate)
                  << std::setw(COL_STATUS) << ScreenUtils::valueOrDash(ts.status) << "\n";
    }
    std::cout << "──────────────────────────────────────────────\n";
}
