#ifndef ADMIN_CONSTANTS_H
#define ADMIN_CONSTANTS_H

#include <string>

namespace AdminConstants
{
    // Common UI Constants
    constexpr int DEFAULT_PANEL_WIDTH = 40;
    constexpr int DEFAULT_PADDING = 2;
    constexpr int ID_COLUMN_WIDTH = 6;
    constexpr int STATUS_COLUMN_WIDTH = 12;
    constexpr int DATE_COLUMN_WIDTH = 16;

    namespace MainMenu
    {
        const std::string MANAGE_EMPLOYEES_OPTION = "1";
        const std::string MANAGE_PROJECTS_OPTION = "2";
        const std::string VIEW_ALLOCATIONS_OPTION = "3";
        const std::string MANAGE_USERS_OPTION = "4";
        const std::string SYSTEM_CONFIG_OPTION = "5";
        const std::string LOGOUT_OPTION = "6";
    }

    namespace Allocations
    {
        constexpr int EMPLOYEE_COLUMN_WIDTH = 20;
        constexpr int PROJECT_COLUMN_WIDTH = 20;
        constexpr int UTILIZATION_COLUMN_WIDTH = 10;
        constexpr int ALLOC_DATE_COLUMN_WIDTH = 12;
    }

    namespace Employees
    {
        const std::string STATUS_BENCH = "BENCH";
        const std::string STATUS_ALLOCATED = "ALLOCATED";

        const std::string CATEGORY_BACKEND = "BACKEND";
        const std::string CATEGORY_FRONTEND = "FRONTEND";
        const std::string CATEGORY_DEVOPS = "DEVOPS";
        const std::string CATEGORY_QA = "QA";
        const std::string CATEGORY_OTHER = "OTHER";

        const std::string PROFICIENCY_BEGINNER = "BEGINNER";
        const std::string PROFICIENCY_INTERMEDIATE = "INTERMEDIATE";
        const std::string PROFICIENCY_ADVANCED = "ADVANCED";

        constexpr int EMPLOYEE_NAME_COLUMN_WIDTH = 20;
        constexpr int DEPARTMENT_COLUMN_WIDTH = 15;

        const std::string OPT_VIEW_ALL = "1";
        const std::string OPT_UPDATE = "2";
        const std::string OPT_DEACTIVATE = "3";
        const std::string OPT_SKILLS = "4";
        const std::string OPT_ASSIGN_MGR = "5";
        const std::string OPT_BACK = "6";

        const std::string ADD_SKILL_OPTION = "1";
        const std::string UPDATE_SKILL_OPTION = "2";
        const std::string REMOVE_SKILL_OPTION = "3";
        const std::string BACK_SKILL_OPTION = "4";
    }

    namespace Projects
    {
        const std::string PROJECT_STATUS_PLANNED = "PLANNED";
        const std::string PROJECT_STATUS_ACTIVE = "ACTIVE";
        const std::string PROJECT_STATUS_ON_HOLD = "ON_HOLD";
        const std::string PROJECT_STATUS_COMPLETED = "COMPLETED";

        const std::string MILESTONE_STATUS_NOT_STARTED = "NOT_STARTED";
        const std::string MILESTONE_STATUS_IN_PROGRESS = "IN_PROGRESS";
        const std::string MILESTONE_STATUS_DONE = "DONE";

        const std::string HEALTH_STATUS_ON_TRACK = "ON_TRACK";
        const std::string HEALTH_FLAG_NORMAL = "NORMAL";

        constexpr int PROJECT_NAME_COLUMN_WIDTH = 24;
        constexpr int MILESTONE_TITLE_COLUMN_WIDTH = 20;
        constexpr int MILESTONE_DATE_COLUMN_WIDTH = 15;

        const std::string OPT_CREATE = "1";
        const std::string OPT_VIEW_ALL = "2";
        const std::string OPT_UPDATE = "3";
        const std::string OPT_MILESTONES = "4";
        const std::string OPT_BACK = "5";

        const std::string ADD_MILESTONE_OPTION = "1";
        const std::string UPDATE_MILESTONE_STATUS_OPTION = "2";
        const std::string BACK_MILESTONE_OPTION = "3";
    }

    namespace Users
    {
        const std::string STATUS_ACTIVE = "ACTIVE";
        const std::string ROLE_ADMIN = "ADMIN";
        const std::string ROLE_MANAGER = "MANAGER";
        const std::string ROLE_EMPLOYEE = "EMPLOYEE";

        const std::string OPT_CREATE_USER = "1";
        const std::string OPT_VIEW_USERS = "2";
        const std::string OPT_RESET_PWD = "3";
        const std::string OPT_DEACTIVATE = "4";
        const std::string OPT_BACK = "5";

        constexpr int USERNAME_COLUMN_WIDTH = 20;
        constexpr int ROLE_COLUMN_WIDTH = 12;
    }

    namespace Config
    {
        constexpr int MAX_HOURS_PER_WEEK = 168;
        constexpr int MIN_HOURS_PER_WEEK = 1;
        constexpr int MIN_SMTP_PORT = 1;
        constexpr int MAX_SMTP_PORT = 65535;

        const std::string OPT_LLM_KEY = "1";
        const std::string OPT_LLM_PROV = "2";
        const std::string OPT_SCHED = "3";
        const std::string OPT_HOURS = "4";
        const std::string OPT_SMTP_EN = "5";
        const std::string OPT_SMTP_SRV = "6";
        const std::string OPT_SMTP_CRED = "7";
        const std::string OPT_SMTP_SND = "8";
        const std::string OPT_SMTP_TLS = "9";
        const std::string OPT_TEST_EMAIL = "10";
        const std::string OPT_BACK = "11";
    }
}

#endif
