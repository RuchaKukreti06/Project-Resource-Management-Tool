#include "employee/SubmitTimesheetScreen.h"
#include "utils/ConsoleInput.h"
#include <sstream>
#include "AuthSession.h"
#include "dto/ProjectDTO.h"
#include "dto/AllocationDTO.h"
#include "dto/ApiResponse.h"
#include "dto/TimesheetDTO.h"
#include "dto/EmployeeDTO.h"
#include <iomanip>
#include "services/EmployeeClientService.h"
#include "services/ProjectClientService.h"
#include "services/AllocationClientService.h"
#include "services/TimesheetClientService.h"
#include "screens/ScreenUtils.h"

SubmitTimesheetScreen::SubmitTimesheetScreen(TimesheetClientService& tsService, AllocationClientService& allocService, ProjectClientService& projService, EmployeeClientService& empService, api::ISessionStore& sessionStore)
    : tsService_(tsService), allocService_(allocService), projService_(projService), empService_(empService), sessionStore_(sessionStore)
{
}

void SubmitTimesheetScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
}

void SubmitTimesheetScreen::show()
{
    displayMenu();
    handleInput();
}

void SubmitTimesheetScreen::handleInput()
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

        std::cout << "Week Start: Enter date (DD-MM-YYYY) or press Enter for last Monday\n";
        std::string weekInput = ConsoleInput::readLine("Week");
        
        std::string weekStart = weekInput;
        if (weekInput.empty())
        {
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
            char buffer[11] = {0};
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &lastMonday);
            weekStart = buffer;
        }
        else if (weekInput.length() == 10 && weekInput[2] == '-' && weekInput[5] == '-')
        {
            weekStart = weekInput.substr(6, 4) + "-" + weekInput.substr(3, 2) + "-" + weekInput.substr(0, 2);
        }        std::cout << "\nChecking your active allocations for this week...\n";

        // Query active projects/allocations
        auto allocRes = allocService_.getEmployeeAllocations(empId);
        std::vector<nlohmann::json> activeAllocs;

        if (allocRes.success)
        {
            for (const auto& alloc : allocRes.data)
            {
                std::string fromD = alloc.fromDate;
                std::string toD = alloc.toDate;
                
                if (fromD <= weekStart && (toD.empty() || toD >= weekStart))
                {
                    activeAllocs.push_back({
                        {"project_id", alloc.projectId},
                        {"project_name", alloc.projectName},
                        {"utilisation_percent", alloc.utilizationPercentage}
                    });
                }
            }
        }

        if (activeAllocs.empty())
        {
            showError("No active allocations found for this week. You cannot submit a timesheet.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        nlohmann::json timesheetLines = nlohmann::json::array();
        int totalHours = 0;

        for (int i = 0; i < (int)activeAllocs.size(); ++i)
        {
            auto alloc = activeAllocs[i];
            int expected = (alloc["utilisation_percent"].get<int>() * 40) / 100;
            std::cout << "\n──────────────────────────────────────────────\n";
            std::cout << "PROJECT " << (i + 1) << " OF " << activeAllocs.size() << " — " << alloc["project_name"].get<std::string>() << "\n";
            std::cout << "  Allocation: " << alloc["utilisation_percent"].get<int>() << "%   |   Expected: " << expected << " hrs max\n";
            std::cout << "──────────────────────────────────────────────\n";

            std::string hrsStr = ConsoleInput::readLine("Hours worked this week");
            auto parsedHours = ScreenUtils::safeParseInt(hrsStr);
            if (!parsedHours) throw std::invalid_argument("Invalid hours format");
            int hours = parsedHours.value();
            totalHours += hours;

            std::cout << "\nWhat did you work on? Select activity tags:\n";
            std::cout << "  1. Backend API Development\n";
            std::cout << "  2. Microservices / Architecture\n";
            std::cout << "  3. Database Design & Queries\n";
            std::cout << "  4. WebSocket / Real-time Features\n";
            std::cout << "  5. Frontend Development\n";
            std::cout << "  6. Code Review / Mentoring\n";
            std::cout << "  7. Bug Fixing\n";
            std::cout << "  8. DevOps / Deployment\n";
            std::cout << "  9. Testing & QA\n";
            std::cout << "  10. Documentation\n";
            std::cout << "  11. Other\n";

            std::string tagsInput = ConsoleInput::readLine("Select tags (comma-separated, e.g., 1,7)");
            
            // Map tag choices to string values
            std::vector<std::string> tags;
            std::stringstream ss(tagsInput);
            std::string item;
            while (std::getline(ss, item, ','))
            {
                if (item == "1") tags.push_back("Backend API Development");
                else if (item == "2") tags.push_back("Microservices / Architecture");
                else if (item == "3") tags.push_back("Database Design & Queries");
                else if (item == "4") tags.push_back("WebSocket / Real-time Features");
                else if (item == "5") tags.push_back("Frontend Development");
                else if (item == "6") tags.push_back("Code Review / Mentoring");
                else if (item == "7") tags.push_back("Bug Fixing");
                else if (item == "8") tags.push_back("DevOps / Deployment");
                else if (item == "9") tags.push_back("Testing & QA");
                else if (item == "10") tags.push_back("Documentation");
                else tags.push_back("Other Development");
            }

            timesheetLines.push_back({
                {"project_id", alloc["project_id"].get<int>()},
                {"hours_worked", hours},
                {"tags", tags}
            });
        }

        std::vector<TimesheetSummaryData> summaryData;

        for (const auto& line : timesheetLines)
        {
            int pId = line["project_id"].get<int>();
            std::string pName = "Proj " + std::to_string(pId);
            for (const auto& a : activeAllocs)
            {
                if (a["project_id"].get<int>() == pId)
                {
                    pName = a["project_name"].get<std::string>();
                    break;
                }
            }

            std::string tagsStr = "[";
            int tCount = 0;
            for (const auto& t : line["tags"])
            {
                if (tCount > 0) tagsStr += ", ";
                tagsStr += t.get<std::string>();
                tCount++;
            }
            tagsStr += "]";

            summaryData.push_back({
                pName,
                std::to_string(line["hours_worked"].get<int>()) + " hrs",
                tagsStr
            });
        }
        
        displayTimesheetSummary(summaryData, totalHours);

        std::cout << "[S] Submit Timesheet      [B] Back\n";
        std::string submitChoice = ConsoleInput::readLine("Choice");
        if (submitChoice == "S" || submitChoice == "s")
        {
            SubmitTimesheetRequest tsReq;
            tsReq.employeeId = empId;
            tsReq.weekStartDate = weekStart;
            tsReq.maxWeeklyHours = 40;
            
            for (const auto& line : timesheetLines)
            {
                TimesheetLineRequest lr;
                lr.projectId = line["project_id"].get<int>();
                lr.hoursLogged = line["hours_worked"].get<int>();
                std::string tagsStr = "";
                int tCount = 0;
                for (const auto& t : line["tags"]) {
                    if (tCount > 0) tagsStr += ",";
                    tagsStr += t.get<std::string>();
                    tCount++;
                }
                lr.activityTag = tagsStr;
                tsReq.lines.push_back(lr);
            }

            auto response = tsService_.submitTimesheet(tsReq);

            if (response.success)
            {
                showSuccess("Timesheet submitted successfully! ✓");
            }
            else
            {
                showError(response.message);
            }
        }
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error during timesheet submission: ") + ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

ScreenDecorator SubmitTimesheetScreen::decorator() const
{
    return ScreenDecorator("SUBMIT TIMESHEET").withWidth(40).withPadding(2);
}

void SubmitTimesheetScreen::displayTimesheetSummary(const std::vector<TimesheetSummaryData>& summaryLines, int totalHours)
{
    std::cout << "\n================ TIMESHEET SUMMARY ================\n";
    std::cout << std::left
              << std::setw(25) << "Project"
              << std::setw(8) << "Hrs"
              << std::setw(40) << "Tags" << "\n";
    std::cout << "─────────────────────────────────────────────────────────────────────────\n";

    for (const auto& line : summaryLines)
    {
        std::cout << std::left
                  << std::setw(25) << ScreenUtils::truncate(ScreenUtils::valueOrDash(line.projectName), 24)
                  << std::setw(8) << ScreenUtils::valueOrDash(line.hours)
                  << std::setw(40) << ScreenUtils::truncate(ScreenUtils::valueOrDash(line.tags), 39) << "\n";
    }
    std::cout << "─────────────────────────────────────────────────────────────────────────\n";
    std::cout << "  Total           " << totalHours << " hrs / 40 hrs max   " << (totalHours <= 40 ? "✓" : "⚠") << "\n\n";
}
