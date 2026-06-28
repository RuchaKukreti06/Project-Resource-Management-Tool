#ifndef MANAGER_CONSTANTS_H
#define MANAGER_CONSTANTS_H

#include <string>

namespace ManagerConstants
{
    const int DEFAULT_PANEL_WIDTH = 40;
    const int DEFAULT_PADDING = 2;
    
    namespace MainMenu
    {
        const std::string OPT_RESOURCE_DASHBOARD = "1";
        const std::string OPT_ALLOCATE_RESOURCE = "2";
        const std::string OPT_MY_PROJECTS = "3";
        const std::string OPT_TIMESHEETS = "4";
        const std::string OPT_AI_ASSISTANT = "5";
        const std::string OPT_LOGOUT = "6";
    }

    namespace MyProjects
    {
        const std::string OPT_AI_RISK_SUMMARY = "A";
        const std::string OPT_BACK = "B";
        const int COL_NO = 6;
        const int COL_PROJECT = 25;
        const int COL_DATE = 15;
        const int COL_HEALTH = 15;
        const int COL_STATUS = 15;
    }

    namespace ResourceDashboard
    {
        const std::string OPT_DRILL_DOWN = "D";
        const std::string OPT_BACK = "B";
    }

    namespace Timesheets
    {
        const std::string OPT_VIEW_DETAIL = "V";
        const std::string OPT_BACK = "B";
        const int COL_EMPLOYEE = 20;
        const int COL_PROJECT = 20;
        const int COL_HOURS = 8;
        const int COL_STATUS = 14;
        const int COL_WEEK_START = 15;
    }

    namespace AllocateResource
    {
        const std::string OPT_FIND_RESOURCE_AI = "1";
        const std::string OPT_ALLOCATE_DIRECTLY = "2";
        const std::string OPT_END_ALLOCATION = "3";
        const std::string OPT_BACK = "4";
    }

    namespace AIAssistant
    {
        const std::string OPT_SKILL_MATCH = "1";
        const std::string OPT_RISK_SUMMARY = "2";
        const std::string OPT_TEAM_BUILDER = "3";
        const std::string OPT_BACK = "4";

        const std::string OPT_GO_ALLOCATE = "A";
    }
}

#endif
