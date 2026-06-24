#ifndef APP_SERVICES_H
#define APP_SERVICES_H

#include <memory>
#include "api/IApiClient.h"
#include "api/ISessionStore.h"
#include "services/AuthClientService.h"
#include "services/UserClientService.h"
#include "services/EmployeeClientService.h"
#include "services/ProjectClientService.h"
#include "services/AllocationClientService.h"
#include "services/TimesheetClientService.h"
#include "services/AiClientService.h"
#include "services/ConfigClientService.h"

struct AppServices
{
    IApiClient& apiClient;
    api::ISessionStore& sessionStore;

    std::unique_ptr<AuthClientService> authService;
    std::unique_ptr<UserClientService> userService;
    std::unique_ptr<EmployeeClientService> employeeService;
    std::unique_ptr<ProjectClientService> projectService;
    std::unique_ptr<AllocationClientService> allocationService;
    std::unique_ptr<TimesheetClientService> timesheetService;
    std::unique_ptr<AiClientService> aiService;
    std::unique_ptr<ConfigClientService> configService;

    AppServices(IApiClient& api, api::ISessionStore& session)
        : apiClient(api), sessionStore(session)
    {
        authService = std::make_unique<AuthClientService>(apiClient);
        userService = std::make_unique<UserClientService>(apiClient);
        employeeService = std::make_unique<EmployeeClientService>(apiClient);
        projectService = std::make_unique<ProjectClientService>(apiClient);
        allocationService = std::make_unique<AllocationClientService>(apiClient);
        timesheetService = std::make_unique<TimesheetClientService>(apiClient);
        aiService = std::make_unique<AiClientService>(apiClient);
        configService = std::make_unique<ConfigClientService>(apiClient);
    }
};

#endif // APP_SERVICES_H
