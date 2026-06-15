#ifndef EMPLOYEE_CONTROLLER_H
#define EMPLOYEE_CONTROLLER_H

#include <httplib.h>

#include "services/EmployeeService.h"

class EmployeeController
{
   public:
    explicit EmployeeController(EmployeeService& employeeService);
    void registerRoutes(httplib::Server& server);

   private:
    EmployeeService& employeeService_;

    void handleGetAllEmployees(const httplib::Request& req, httplib::Response& res);
    void handleGetTeamEmployees(const httplib::Request& req, httplib::Response& res);
    void handleCreateEmployee(const httplib::Request& req, httplib::Response& res);
    void handleUpdateEmployee(const httplib::Request& req, httplib::Response& res);
    void handleDeactivateEmployee(const httplib::Request& req, httplib::Response& res);
    void handleGetEmployeeSkills(const httplib::Request& req, httplib::Response& res);
    void handleAddSkill(const httplib::Request& req, httplib::Response& res);
    void handleUpdateSkill(const httplib::Request& req, httplib::Response& res);
    void handleRemoveSkill(const httplib::Request& req, httplib::Response& res);
};

#endif
