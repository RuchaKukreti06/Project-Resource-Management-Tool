#include "dto/DTOMapper.h"



UserResponse DTOMapper::mapToUserResponse(const User& user)
{
    UserResponse r;
    r.id = user.id;
    r.username = user.username;
    r.fullName = user.fullName;
    r.email = user.email;
    r.role = user.role;
    r.department = user.department;
    r.designation = user.designation;
    r.managerId = user.managerId;
    r.status = user.status;
    r.forcePasswordChange = user.forcePasswordChange;
    r.createdAt = user.createdAt;
    r.updatedAt = user.updatedAt;
    return r;
}

ProjectResponse DTOMapper::mapToProjectResponse(const Project& project)
{
    ProjectResponse r;
    r.id = project.id;
    r.name = project.name;
    r.description = project.description;
    r.startDate = project.startDate;
    r.endDate = project.endDate;
    r.totalStoryPoints = project.totalStoryPoints;
    r.status = project.status;
    r.healthStatus = project.healthStatus;
    r.managerId = project.managerId;
    r.createdAt = project.createdAt;
    return r;
}

EmployeeResponse DTOMapper::mapToEmployeeResponse(const Employee& emp)
{
    EmployeeResponse r;
    r.id = emp.id;
    r.userId = emp.user_id;
    r.fullName = emp.fullName;
    r.email = emp.email;
    r.department = emp.department;
    r.designation = emp.designation;
    r.status = emp.status;
    r.isActive = emp.isActive;
    r.totalUtilisation = emp.totalUtilisation;
    return r;
}

AllocationResponse DTOMapper::mapToAllocationResponse(const Allocation& alloc)
{
    AllocationResponse r;
    r.id = alloc.id;
    r.employeeId = alloc.employeeId;
    r.projectId = alloc.projectId;
    r.utilizationPercentage = alloc.utilizationPercentage;
    r.fromDate = alloc.fromDate;
    r.toDate = alloc.toDate;
    return r;
}

TimesheetResponse DTOMapper::mapToTimesheetResponse(const Timesheet& ts)
{
    TimesheetResponse r;
    r.id = ts.id;
    r.employeeId = ts.employeeId;
    r.weekStartDate = ts.weekStartDate;
    r.submittedAt = ts.submittedAt;
    r.status = ts.status;
    r.totalHours = ts.totalHours;
    return r;
}

std::vector<UserResponse> DTOMapper::mapToUserResponse(const std::vector<User>& users)
{
    std::vector<UserResponse> result;
    result.reserve(users.size());
    for (const auto& u : users) result.push_back(mapToUserResponse(u));
    return result;
}

std::vector<ProjectResponse> DTOMapper::mapToProjectResponse(const std::vector<Project>& projects)
{
    std::vector<ProjectResponse> result;
    result.reserve(projects.size());
    for (const auto& p : projects) result.push_back(mapToProjectResponse(p));
    return result;
}

std::vector<EmployeeResponse> DTOMapper::mapToEmployeeResponse(const std::vector<Employee>& emps)
{
    std::vector<EmployeeResponse> result;
    result.reserve(emps.size());
    for (const auto& e : emps) result.push_back(mapToEmployeeResponse(e));
    return result;
}

std::vector<AllocationResponse> DTOMapper::mapToAllocationResponse(const std::vector<Allocation>& allocs)
{
    std::vector<AllocationResponse> result;
    result.reserve(allocs.size());
    for (const auto& a : allocs) result.push_back(mapToAllocationResponse(a));
    return result;
}

std::vector<TimesheetResponse> DTOMapper::mapToTimesheetResponse(const std::vector<Timesheet>& ts)
{
    std::vector<TimesheetResponse> result;
    result.reserve(ts.size());
    for (const auto& t : ts) result.push_back(mapToTimesheetResponse(t));
    return result;
}

MilestoneResponse DTOMapper::mapToMilestoneResponse(const Milestone& ms)
{
    MilestoneResponse r;
    r.id = ms.id;
    r.projectId = ms.projectId;
    r.title = ms.title;
    r.dueDate = ms.dueDate;
    r.storyPoints = ms.storyPoints;
    r.status = ms.status;
    r.healthFlag = ms.healthFlag;
    return r;
}

EmployeeSkillResponse DTOMapper::mapToEmployeeSkillResponse(const EmployeeSkillView& skill)
{
    EmployeeSkillResponse r;
    r.skillId = skill.skillId;
    r.skillName = skill.skillName;
    r.category = skill.category;
    r.proficiency = skill.proficiencyLevel;
    return r;
}

TeamTimesheetResponse DTOMapper::mapToTeamTimesheetResponse(const TeamTimesheetRow& row)
{
    TeamTimesheetResponse r;
    r.employeeName = row.employeeName;
    r.employeeId = row.employeeId;
    r.projectId = row.projectId;
    r.projectName = row.projectName;
    r.hours = row.hours;
    r.status = row.status;
    return r;
}

TimesheetDetailResponse DTOMapper::mapToTimesheetDetailResponse(const TimesheetDetailRow& row)
{
    TimesheetDetailResponse r;
    r.projectId = row.projectId;
    r.projectName = row.projectName;
    r.hours = row.hours;
    r.tags = row.tags;
    return r;
}

std::vector<MilestoneResponse> DTOMapper::mapToMilestoneResponse(const std::vector<Milestone>& ms)
{
    std::vector<MilestoneResponse> result;
    result.reserve(ms.size());
    for (const auto& m : ms) result.push_back(mapToMilestoneResponse(m));
    return result;
}

std::vector<EmployeeSkillResponse> DTOMapper::mapToEmployeeSkillResponse(const std::vector<EmployeeSkillView>& skills)
{
    std::vector<EmployeeSkillResponse> result;
    result.reserve(skills.size());
    for (const auto& s : skills) result.push_back(mapToEmployeeSkillResponse(s));
    return result;
}

std::vector<TeamTimesheetResponse> DTOMapper::mapToTeamTimesheetResponse(const std::vector<TeamTimesheetRow>& rows)
{
    std::vector<TeamTimesheetResponse> result;
    result.reserve(rows.size());
    for (const auto& r : rows) result.push_back(mapToTeamTimesheetResponse(r));
    return result;
}

std::vector<TimesheetDetailResponse> DTOMapper::mapToTimesheetDetailResponse(const std::vector<TimesheetDetailRow>& rows)
{
    std::vector<TimesheetDetailResponse> result;
    result.reserve(rows.size());
    for (const auto& r : rows) result.push_back(mapToTimesheetDetailResponse(r));
    return result;
}
