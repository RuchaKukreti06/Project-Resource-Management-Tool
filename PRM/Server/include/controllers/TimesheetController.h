#pragma once

#include "httplib.h"

#include "services/interfaces/ITimesheetService.h"
#include "services/interfaces/INotificationService.h"

#include "services/interfaces/ITokenService.h"
#include "services/interfaces/IEmployeeService.h"

class TimesheetController
{
   public:
    explicit TimesheetController(ITimesheetService& timesheetService,
                                 INotificationService& notificationService,
                                 ITokenService& tokenService,
                                 IEmployeeService& employeeService);
    void registerRoutes(httplib::Server& server) const;

   private:
    ITimesheetService& timesheetService_;
    INotificationService& notificationService_;
    ITokenService& tokenService_;
    IEmployeeService& employeeService_;
};
