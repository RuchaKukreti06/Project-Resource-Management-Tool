#pragma once

#include <vector>
#include <nlohmann/json.hpp>

#include "User.h"
#include "Project.h"
#include "Employee.h"
#include "Allocations.h"
#include "Timesheet.h"
#include "Milestones.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/ITimesheetRepository.h"

#include "dto/response/UserResponse.h"
#include "dto/response/ProjectResponse.h"
#include "dto/response/EmployeeResponse.h"
#include "dto/response/AllocationResponse.h"
#include "dto/response/TimesheetResponse.h"
#include "dto/response/LoginResponse.h"
#include "dto/response/RegisterResponse.h"
#include "dto/response/MilestoneResponse.h"
#include "dto/response/EmployeeSkillResponse.h"
#include "dto/response/TeamTimesheetResponse.h"
#include "dto/response/TimesheetDetailResponse.h"



class DTOMapper
{
public:
    static UserResponse mapToUserResponse(const User& user);
    static ProjectResponse mapToProjectResponse(const Project& project);
    static EmployeeResponse mapToEmployeeResponse(const Employee& emp);
    static AllocationResponse mapToAllocationResponse(const Allocation& alloc);
    static TimesheetResponse mapToTimesheetResponse(const Timesheet& ts);
    static MilestoneResponse mapToMilestoneResponse(const Milestone& ms);
    static EmployeeSkillResponse mapToEmployeeSkillResponse(const EmployeeSkillView& skill);
    static TeamTimesheetResponse mapToTeamTimesheetResponse(const TeamTimesheetRow& row);
    static TimesheetDetailResponse mapToTimesheetDetailResponse(const TimesheetDetailRow& row);

    static std::vector<UserResponse> mapToUserResponse(const std::vector<User>& users);
    static std::vector<ProjectResponse> mapToProjectResponse(const std::vector<Project>& projects);
    static std::vector<EmployeeResponse> mapToEmployeeResponse(const std::vector<Employee>& emps);
    static std::vector<AllocationResponse> mapToAllocationResponse(const std::vector<Allocation>& allocs);
    static std::vector<TimesheetResponse> mapToTimesheetResponse(const std::vector<Timesheet>& ts);
    static std::vector<MilestoneResponse> mapToMilestoneResponse(const std::vector<Milestone>& ms);
    static std::vector<EmployeeSkillResponse> mapToEmployeeSkillResponse(const std::vector<EmployeeSkillView>& skills);
    static std::vector<TeamTimesheetResponse> mapToTeamTimesheetResponse(const std::vector<TeamTimesheetRow>& rows);
    static std::vector<TimesheetDetailResponse> mapToTimesheetDetailResponse(const std::vector<TimesheetDetailRow>& rows);
};
