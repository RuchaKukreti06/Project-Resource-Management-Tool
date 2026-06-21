#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>

#include "services/SchedulerService.h"
#include "services/EmployeeService.h"
#include "services/ProjectService.h"
#include "services/AllocationService.h"
#include "services/TimesheetService.h"

#include "mocks/MockEmployeeRepository.h"
#include "mocks/MockUserRepository.h"
#include "mocks/MockProjectRepository.h"
#include "mocks/MockAllocationRepository.h"
#include "mocks/MockTimesheetRepository.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;

class SchedulerServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockEmployeeRepo = std::make_shared<MockEmployeeRepository>();
        mockUserRepo = std::make_shared<MockUserRepository>();
        mockProjectRepo = std::make_shared<MockProjectRepository>();
        mockAllocRepo = std::make_shared<MockAllocationRepository>();
        mockTimesheetRepo = std::make_shared<MockTimesheetRepository>();

        auto empSvc = std::make_shared<EmployeeService>(mockEmployeeRepo, mockUserRepo, mockAllocRepo);
        auto projSvc = std::make_shared<ProjectService>(mockProjectRepo, mockUserRepo);
        auto allocSvc = std::make_shared<AllocationService>(mockAllocRepo, mockEmployeeRepo, mockProjectRepo);
        auto tsSvc = std::make_shared<TimesheetService>(mockTimesheetRepo, mockEmployeeRepo, mockAllocRepo, nullptr, nullptr);

        schedulerService = std::make_unique<SchedulerService>(empSvc, projSvc, allocSvc, tsSvc);
    }

    std::shared_ptr<MockEmployeeRepository> mockEmployeeRepo;
    std::shared_ptr<MockUserRepository> mockUserRepo;
    std::shared_ptr<MockProjectRepository> mockProjectRepo;
    std::shared_ptr<MockAllocationRepository> mockAllocRepo;
    std::shared_ptr<MockTimesheetRepository> mockTimesheetRepo;
    
    std::unique_ptr<SchedulerService> schedulerService;
};

TEST_F(SchedulerServiceTests, FlagMissedTimesheets_Success)
{
    std::vector<int> missedEmpIds = { 1, 2 };
    EXPECT_CALL(*mockTimesheetRepo, getEmployeesWithMissedTimesheets("2024-01-01")).WillOnce(Return(missedEmpIds));

    // mockEmployeeRepo->getEmployeeById will return nullopt by default if employee not in map
    EXPECT_NO_THROW(schedulerService->flagMissedTimesheets("2024-01-01"));
}


