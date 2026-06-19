#pragma once

#include "httplib.h"

#include "services/interfaces/ITimesheetService.h"
#include "services/interfaces/INotificationService.h"

class TimesheetController
{
   public:
    explicit TimesheetController(ITimesheetService& timesheetService,
                                 INotificationService& notificationService);
    void registerRoutes(httplib::Server& server) const;

   private:
    ITimesheetService& timesheetService_;
    INotificationService& notificationService_;
};
