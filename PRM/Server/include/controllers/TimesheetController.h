#pragma once

#include "httplib.h"

#include "services/interfaces/ITimesheetService.h"

class TimesheetController
{
   public:
    explicit TimesheetController(ITimesheetService& timesheetService);
    void registerRoutes(httplib::Server& server) const;

   private:
    ITimesheetService& timesheetService_;
};
