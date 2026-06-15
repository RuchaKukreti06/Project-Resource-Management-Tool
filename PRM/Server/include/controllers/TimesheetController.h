#pragma once

#include "httplib.h"

#include "services/TimesheetService.h"

class TimesheetController
{
   public:
    explicit TimesheetController(TimesheetService& timesheetService);
    void registerRoutes(httplib::Server& server) const;

   private:
    TimesheetService& timesheetService_;
};
