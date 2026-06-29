#include "admin/ManageProjectsScreen.h"
#include "dto/ApiResponse.h"
#include "dto/ProjectDTO.h"
#include <iomanip>
#include <iostream>
#include "api/ApiException.h"
#include "services/ProjectClientService.h"
#include "utils/ConsoleInput.h"
#include "screens/ScreenUtils.h"
#include "utils/DateUtils.h"

using namespace AdminConstants;
using namespace AdminConstants::Projects;

ManageProjectsScreen::ManageProjectsScreen(ProjectClientService& projService)
    : projService_(projService)
{
}

void ManageProjectsScreen::displayMenu()
{
    decorator().render();
    std::cout << OPT_CREATE << ". Create Project\n";
    std::cout << OPT_VIEW_ALL << ". View All Projects\n";
    std::cout << OPT_UPDATE << ". Update Project Details\n";
    std::cout << OPT_MILESTONES << ". Manage Milestones\n";
    std::cout << OPT_BACK << ". Back\n";
}

void ManageProjectsScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput();
    }
}

void ManageProjectsScreen::handleInput()
{
    std::string choice = ConsoleInput::readLine("Enter option");
    if (choice == OPT_CREATE)
    {
        createProject();
    }
    else if (choice == OPT_VIEW_ALL)
    {
        viewAllProjects();
    }
    else if (choice == OPT_UPDATE)
    {
        updateProjectDetails();
    }
    else if (choice == OPT_MILESTONES)
    {
        manageMilestones();
    }
    else if (choice == OPT_BACK || ScreenUtils::equalsIgnoreCase(choice, "B"))
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1–5.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

bool ManageProjectsScreen::isValidProjectName(const std::string& name)
{
    for (unsigned char c : name) {
        if (c < 0x20 || c == 0x7F || c == '\\') {
            showError("Project name contains invalid characters (no backslashes or control chars allowed).");
            return false;
        }
    }
    return true;
}

bool ManageProjectsScreen::isValidProjectDateRange(const std::string& start, const std::string& end)
{
    if (!start.empty() && !end.empty() && end <= start) {
        showError("End Date must be strictly after Start Date.");
        return false;
    }
    return true;
}

std::optional<int> ManageProjectsScreen::promptForStoryPoints(const std::string& prompt)
{
    while (true)
    {
        std::string totalSP = ConsoleInput::readLine(prompt);
        if (totalSP.empty()) return 0;
        auto parsedSP = ScreenUtils::safeParseInt(totalSP);
        if (!parsedSP)
        {
            showError("Invalid story points format");
            continue;
        }
        if (parsedSP.value() < 0 || parsedSP.value() > 10000)
        {
            showError("Total story points must be between 0 and 10000.");
            continue;
        }
        return parsedSP.value();
    }
}

std::optional<int> ManageProjectsScreen::promptForManagerUserId(const std::string& prompt)
{
    return ScreenUtils::promptForInt(prompt, "Invalid manager ID format");
}

std::optional<std::string> ManageProjectsScreen::promptForProjectStatus(const std::string& currentStatus)
{
    std::cout << "Status:\n";
    std::cout << "1. PLANNED\n";
    std::cout << "2. ACTIVE\n";
    std::cout << "3. ON_HOLD\n";
    std::cout << "4. COMPLETED\n";
    std::string promptStr = currentStatus.empty() ? "Choice" : ("Choice (Press Enter to keep: " + currentStatus + ")");
    std::string statusChoice = ConsoleInput::readLine(promptStr);
    
    if (statusChoice.empty() && !currentStatus.empty())
    {
        return currentStatus;
    }
    
    if (statusChoice == "1") return PROJECT_STATUS_PLANNED;
    if (statusChoice == "2") return PROJECT_STATUS_ACTIVE;
    if (statusChoice == "3") return PROJECT_STATUS_ON_HOLD;
    if (statusChoice == "4") return PROJECT_STATUS_COMPLETED;
    
    showError("Invalid status choice.");
    ConsoleInput::waitForEnter("Press Enter to continue\n");
    return std::nullopt;
}

std::optional<std::string> ManageProjectsScreen::promptForMilestoneStatus()
{
    std::cout << "New Status:\n";
    std::cout << "1. NOT_STARTED\n";
    std::cout << "2. IN_PROGRESS\n";
    std::cout << "3. DONE\n";
    std::string stChoice = ConsoleInput::readLine("Choice");
    if (stChoice == "1") return MILESTONE_STATUS_NOT_STARTED;
    if (stChoice == "2") return MILESTONE_STATUS_IN_PROGRESS;
    if (stChoice == "3") return MILESTONE_STATUS_DONE;
    
    showError("Invalid milestone status choice.");
    ConsoleInput::waitForEnter("Press Enter to continue\n");
    return std::nullopt;
}

std::optional<std::string> ManageProjectsScreen::promptForProjectName()
{
    std::string name = ConsoleInput::readLine("Project Name");
    if (name.empty())
    {
        showError("Project name cannot be empty.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return std::nullopt;
    }
    if (!isValidProjectName(name)) return std::nullopt;
    return name;
}

std::optional<std::pair<std::string, std::string>> ManageProjectsScreen::promptForProjectDates(const std::string& status)
{
    std::string startDate = ScreenUtils::readValidDate("Start Date (YYYY-MM-DD)", true);
    std::string endDate = ScreenUtils::readValidDate("End Date (YYYY-MM-DD)", true);

    if (!isValidProjectDateRange(startDate, endDate)) 
    {
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return std::nullopt;
    }

    if (status == PROJECT_STATUS_COMPLETED)
    {
        if (endDate.empty())
        {
            showError("A completed project must have an end date.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return std::nullopt;
        }
        std::string err = DateUtils::validateDateYYYYMMDD(endDate, false);
        if (!err.empty())
        {
            showError("A completed project cannot have an end date in the future.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return std::nullopt;
        }
    }
    return std::make_pair(startDate, endDate);
}

std::optional<std::pair<std::string, std::string>> ManageProjectsScreen::promptForUpdatedBasicDetails(const ProjectDTO& currentProj)
{
    std::string newName = ConsoleInput::readLine("Project Name (" + currentProj.name + ")");
    std::string newDesc = ConsoleInput::readLine("Description (" + currentProj.description + ")");

    if (newName.empty()) newName = currentProj.name;
    if (newDesc.empty()) newDesc = currentProj.description;

    if (!isValidProjectName(newName)) return std::nullopt;

    return std::make_pair(newName, newDesc);
}

std::optional<std::pair<std::string, std::string>> ManageProjectsScreen::promptForUpdatedDates(const ProjectDTO& currentProj, const std::string& status)
{
    std::string newStart = ScreenUtils::readOptionalDateForUpdate("Start Date", true, currentProj.startDate);
    std::string newEnd = ScreenUtils::readOptionalDateForUpdate("End Date", true, currentProj.endDate);

    if (!isValidProjectDateRange(newStart, newEnd)) 
    {
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return std::nullopt;
    }

    if (newStart.empty()) newStart = currentProj.startDate;
    if (newEnd.empty()) newEnd = currentProj.endDate;
    
    if (status == PROJECT_STATUS_COMPLETED)
    {
        if (newEnd.empty())
        {
            showError("A completed project must have an end date.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return std::nullopt;
        }
        std::string err = DateUtils::validateDateYYYYMMDD(newEnd, false);
        if (!err.empty())
        {
            showError("A completed project cannot have an end date in the future.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return std::nullopt;
        }
    }
    return std::make_pair(newStart, newEnd);
}

std::optional<int> ManageProjectsScreen::promptForUpdatedManager(const ProjectDTO& currentProj)
{
    std::string newMgrId = ConsoleInput::readLine("Assign Manager (" + std::to_string(currentProj.managerId) + ")");
    int managerId = currentProj.managerId;
    if (!newMgrId.empty())
    {
        auto mgrIdParsed = ScreenUtils::safeParseInt(newMgrId);
        if (mgrIdParsed)
        {
            managerId = mgrIdParsed.value();
        }
        else
        {
            showError("Invalid manager ID format");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return std::nullopt;
        }
    }
    return managerId;
}

std::optional<CreateProjectRequest> ManageProjectsScreen::promptForNewProjectDetails()
{
    std::cout << "\n========== CREATE PROJECT ==========\n";
    auto nameOpt = promptForProjectName();
    if (!nameOpt) return std::nullopt;

    std::string desc = ConsoleInput::readLine("Description");

    auto statusOpt = promptForProjectStatus(PROJECT_STATUS_PLANNED);
    if (!statusOpt) return std::nullopt;

    auto datesOpt = promptForProjectDates(statusOpt.value());
    if (!datesOpt) return std::nullopt;

    auto spOpt = promptForStoryPoints("Total Story Points (leave blank for 0)");
    if (!spOpt) return std::nullopt;

    auto mgrIdOpt = promptForManagerUserId("Assign Manager (Enter Manager User ID)");
    if (!mgrIdOpt) return std::nullopt;

    CreateProjectRequest createProjectRequest;
    createProjectRequest.name = nameOpt.value();
    createProjectRequest.description = desc;
    createProjectRequest.startDate = datesOpt.value().first;
    createProjectRequest.endDate = datesOpt.value().second;
    createProjectRequest.status = statusOpt.value();
    createProjectRequest.totalStoryPoints = spOpt.value();
    createProjectRequest.healthStatus = HEALTH_STATUS_ON_TRACK;
    createProjectRequest.managerId = mgrIdOpt.value();

    return createProjectRequest;
}

std::optional<CreateProjectRequest> ManageProjectsScreen::promptForProjectUpdate(const ProjectDTO& currentProj)
{
    std::cout << "\n── " << currentProj.name << " ─────────────────────────────────\n";
    auto basicDetailsOpt = promptForUpdatedBasicDetails(currentProj);
    if (!basicDetailsOpt) return std::nullopt;

    auto newStatusOpt = promptForProjectStatus(currentProj.status);
    if (!newStatusOpt) return std::nullopt;

    auto newMgrIdOpt = promptForUpdatedManager(currentProj);
    if (!newMgrIdOpt) return std::nullopt;

    auto datesOpt = promptForUpdatedDates(currentProj, newStatusOpt.value());
    if (!datesOpt) return std::nullopt;

    CreateProjectRequest projectUpdateRequestPayload;
    projectUpdateRequestPayload.name = basicDetailsOpt.value().first;
    projectUpdateRequestPayload.description = basicDetailsOpt.value().second;
    projectUpdateRequestPayload.startDate = datesOpt.value().first;
    projectUpdateRequestPayload.endDate = datesOpt.value().second;
    projectUpdateRequestPayload.status = newStatusOpt.value();
    projectUpdateRequestPayload.totalStoryPoints = currentProj.totalStoryPoints;
    projectUpdateRequestPayload.healthStatus = currentProj.healthStatus;
    projectUpdateRequestPayload.managerId = newMgrIdOpt.value();
    
    return projectUpdateRequestPayload;
}

void ManageProjectsScreen::createProject()
{
    try
    {
        auto projectRequestOpt = promptForNewProjectDetails();
        if (!projectRequestOpt) return;

        auto createResponse = projService_.createProject(projectRequestOpt.value());

        if (createResponse.success)
        {
            showSuccess("Project created successfully. ✓");
        }
        else
        {
            showError(createResponse.message);
        }
        ConsoleInput::waitForEnter("Press Enter to continue\n");
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

void ManageProjectsScreen::viewAllProjects()
{
    try
    {
        auto response = projService_.viewAllProjects();
        if (!response.success)
        {
            showError(response.message);
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        auto projects = response.data;
        displayProjects(projects);
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

void ManageProjectsScreen::updateProjectDetails()
{
    try
    {
        auto projIdOpt = promptForProjectId("Enter Project ID");
        if (!projIdOpt) return;
        
        auto response = projService_.getProject(projIdOpt.value());
        if (!response.success || !response.data)
        {
            showError("Project not found.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        auto targetProj = *response.data;
        auto projectRequestOpt = promptForProjectUpdate(targetProj);
        if (!projectRequestOpt) return;

        auto updateResponse = projService_.updateProject(targetProj.id, projectRequestOpt.value());

        if (updateResponse.success)
        {
            showSuccess("Project details updated. ✓");
        }
        else
        {
            showError(updateResponse.message);
        }
        ConsoleInput::waitForEnter("Press Enter to continue\n");
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

void ManageProjectsScreen::manageMilestones()
{
    try
    {
        auto projectIdOpt = promptForProjectId("Enter Project ID");
        if (!projectIdOpt) return;
        int projectId = projectIdOpt.value();
        
        while (true)
        {
            auto milestonesResponse = projService_.getProjectMilestones(projectId);
            if (!milestonesResponse.success)
            {
                showError("Failed to fetch project milestones.");
                ConsoleInput::waitForEnter("Press Enter to continue\n");
                return;
            }

            auto milestones = milestonesResponse.data;
            displayMilestones(milestones);

            std::cout << ADD_MILESTONE_OPTION << ". Add Milestone\n";
            std::cout << UPDATE_MILESTONE_STATUS_OPTION << ". Update Milestone Status\n";
            std::cout << BACK_MILESTONE_OPTION << ". Back\n";
            std::string selectedOption = ConsoleInput::readLine("Enter option");

            if (!handleMilestoneOption(selectedOption, projectId, milestones))
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

bool ManageProjectsScreen::handleMilestoneOption(const std::string& selectedOption, int projectId, const std::vector<MilestoneDTO>& milestones)
{
    if (selectedOption == ADD_MILESTONE_OPTION)
    {
        handleAddMilestone(projectId);
    }
    else if (selectedOption == UPDATE_MILESTONE_STATUS_OPTION)
    {
        handleUpdateMilestoneStatus(milestones);
    }
    else if (selectedOption == BACK_MILESTONE_OPTION || ScreenUtils::equalsIgnoreCase(selectedOption, "B"))
    {
        return false;
    }
    else
    {
        showError("Invalid option. Please enter 1–3.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    return true;
}

ScreenDecorator ManageProjectsScreen::decorator() const
{
    return ScreenDecorator("MANAGE PROJECTS").withWidth(DEFAULT_PANEL_WIDTH).withPadding(DEFAULT_PADDING);
}

void ManageProjectsScreen::displayProjects(const std::vector<ProjectDTO>& projects)
{
    std::cout << "\n================ ALL PROJECTS ================\n";
    std::cout << std::left
              << std::setw(ID_COLUMN_WIDTH) << "ID"
              << std::setw(PROJECT_NAME_COLUMN_WIDTH) << "Name"
              << std::setw(DATE_COLUMN_WIDTH) << "End Date"
              << std::setw(STATUS_COLUMN_WIDTH) << "Status" << "\n";
    ScreenUtils::printDivider();

    for (const auto& proj : projects)
    {
        std::cout << std::left
                  << std::setw(ID_COLUMN_WIDTH) << proj.id
                  << std::setw(PROJECT_NAME_COLUMN_WIDTH) << ScreenUtils::truncate(ScreenUtils::valueOrDash(proj.name), PROJECT_NAME_COLUMN_WIDTH - 1)
                  << std::setw(DATE_COLUMN_WIDTH) << ScreenUtils::valueOrDash(proj.endDate)
                  << std::setw(STATUS_COLUMN_WIDTH) << ScreenUtils::valueOrDash(proj.status) << "\n";
    }
    ScreenUtils::printDivider();
}

void ManageProjectsScreen::displayMilestones(const std::vector<MilestoneDTO>& milestones)
{
    std::cout << "\n================= MILESTONES =================\n";
    std::cout << std::left
              << std::setw(ID_COLUMN_WIDTH) << "#"
              << std::setw(MILESTONE_TITLE_COLUMN_WIDTH) << "Title"
              << std::setw(MILESTONE_DATE_COLUMN_WIDTH) << "Due Date"
              << std::setw(STATUS_COLUMN_WIDTH) << "Status" << "\n";
    ScreenUtils::printDivider();

    int idx = 1;
    for (const auto& ms : milestones)
    {
        std::cout << std::left
                  << std::setw(ID_COLUMN_WIDTH) << idx++
                  << std::setw(MILESTONE_TITLE_COLUMN_WIDTH) << ScreenUtils::truncate(ScreenUtils::valueOrDash(ms.title), MILESTONE_TITLE_COLUMN_WIDTH - 1)
                  << std::setw(MILESTONE_DATE_COLUMN_WIDTH) << ScreenUtils::valueOrDash(ms.dueDate)
                  << std::setw(STATUS_COLUMN_WIDTH) << ScreenUtils::valueOrDash(ms.status) << "\n";
    }
    ScreenUtils::printDivider();
}

std::optional<int> ManageProjectsScreen::promptForProjectId(const std::string& prompt)
{
    return ScreenUtils::promptForInt(prompt, "Invalid ID format.");
}

void ManageProjectsScreen::handleAddMilestone(int projectId)
{
    std::string title = ConsoleInput::readLine("Milestone Title");
    std::string dueDate = ScreenUtils::readValidDate("Due Date (YYYY-MM-DD)", true);

    AddMilestoneRequest addMilestoneRequest;
    addMilestoneRequest.title = title;
    addMilestoneRequest.dueDate = dueDate;
    addMilestoneRequest.storyPoints = 0;
    addMilestoneRequest.status = MILESTONE_STATUS_NOT_STARTED;
    addMilestoneRequest.healthFlag = HEALTH_FLAG_NORMAL;

    auto createResponse = projService_.addMilestone(projectId, addMilestoneRequest);

    if (createResponse.success)
    {
        showSuccess("Milestone added successfully. ✓");
    }
    else
    {
        showError(createResponse.message);
    }
    ConsoleInput::waitForEnter("Press Enter to continue\n");
}

void ManageProjectsScreen::handleUpdateMilestoneStatus(const std::vector<MilestoneDTO>& milestones)
{
    std::string inputStr = ConsoleInput::readLine("Enter Milestone #");
    auto parsedInput = ScreenUtils::safeParseInt(inputStr);
    if (!parsedInput)
    {
        showError("Invalid milestone number format");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    int itemNum = parsedInput.value();
    if (itemNum < 1 || itemNum > (int)milestones.size())
    {
        showError("Invalid milestone number.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    int milestoneId = milestones[itemNum - 1].id;

    auto statusOpt = promptForMilestoneStatus();
    if (!statusOpt) return;

    auto updateResponse = projService_.updateMilestoneStatus(milestoneId, statusOpt.value());

    if (updateResponse.success)
    {
        showSuccess("Milestone updated. ✓");
    }
    else
    {
        showError(updateResponse.message);
    }
    ConsoleInput::waitForEnter("Press Enter to continue\n");
}
