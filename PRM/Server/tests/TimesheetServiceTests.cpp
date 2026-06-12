#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>

#include "services/TimesheetService.h"
#include "mocks/MockTimesheetRepository.h"
#include "mocks/MockEmployeeRepository.h"
#include "mocks/MockAllocationRepository.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;

class TimesheetServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockTimesheetRepo = std::make_shared<MockTimesheetRepository>();
        mockEmployeeRepo = std::make_shared<MockEmployeeRepository>();
        mockAllocRepo = std::make_shared<MockAllocationRepository>();
        timesheetService = std::make_unique<TimesheetService>(mockTimesheetRepo, mockEmployeeRepo, mockAllocRepo);
    }

    std::shared_ptr<MockTimesheetRepository> mockTimesheetRepo;
    std::shared_ptr<MockEmployeeRepository> mockEmployeeRepo;
    std::shared_ptr<MockAllocationRepository> mockAllocRepo;
    std::unique_ptr<TimesheetService> timesheetService;
};

TEST_F(TimesheetServiceTests, SubmitTimesheet_AlreadyExists_Fails)
{
    Employee mockEmp; mockEmp.id = 1; mockEmp.user_id = 101; mockEmp.isActive = true; mockEmp.status = "ACTIVE";
    mockEmployeeRepo->employees[1] = mockEmp;
    EXPECT_CALL(*mockTimesheetRepo, existsTimesheetForWeek(1, "2024-01-01")).WillOnce(Return(true));

    std::string message;
    std::vector<TimesheetLineInput> lines;
    bool result = timesheetService->submitTimesheet(1, "2024-01-01", lines, 40, message);
    EXPECT_FALSE(result);
    EXPECT_EQ(message, "Timesheet already exists for this week.");
}

TEST_F(TimesheetServiceTests, SubmitTimesheet_ExceedsMaxHours_Fails)
{
    Employee mockEmp; mockEmp.id = 1; mockEmp.user_id = 101; mockEmp.isActive = true; mockEmp.status = "ACTIVE";
    mockEmployeeRepo->employees[1] = mockEmp;
    EXPECT_CALL(*mockTimesheetRepo, existsTimesheetForWeek(1, "2024-01-01")).WillOnce(Return(false));

    Allocation alloc; alloc.id = 1; alloc.employeeId = 1; alloc.projectId = 1; alloc.utilizationPercentage = 100; alloc.fromDate = "2024-01-01"; alloc.toDate = "2024-12-31";
    std::vector<Allocation> allocs = { alloc };
    EXPECT_CALL(*mockTimesheetRepo, getActiveAllocationsForWeek(1, "2024-01-01", _)).WillOnce(Return(allocs));

    std::string message;
    std::vector<TimesheetLineInput> lines = { {1, 50, {"tag"}} };
    bool result = timesheetService->submitTimesheet(1, "2024-01-01", lines, 40, message);
    EXPECT_FALSE(result);
    EXPECT_EQ(message, "Project hours exceed allowed allocation limit.");
}

TEST_F(TimesheetServiceTests, SubmitTimesheet_Success)
{
    Employee mockEmp; mockEmp.id = 1; mockEmp.user_id = 101; mockEmp.isActive = true; mockEmp.status = "ACTIVE";
    mockEmployeeRepo->employees[1] = mockEmp;
    EXPECT_CALL(*mockTimesheetRepo, existsTimesheetForWeek(1, "2024-01-01")).WillOnce(Return(false));
    
    Allocation alloc; alloc.id = 1; alloc.employeeId = 1; alloc.projectId = 1; alloc.utilizationPercentage = 100; alloc.fromDate = "2024-01-01"; alloc.toDate = "2024-12-31";
    std::vector<Allocation> allocs = { alloc };
    EXPECT_CALL(*mockTimesheetRepo, getActiveAllocationsForWeek(1, "2024-01-01", _)).WillOnce(Return(allocs));
    
    EXPECT_CALL(*mockTimesheetRepo, createTimesheetWithLines(1, "2024-01-01", _)).WillOnce(Return(true));

    std::string message;
    std::vector<TimesheetLineInput> lines = { {1, 40, {"tag"}} };
    bool result = timesheetService->submitTimesheet(1, "2024-01-01", lines, 40, message);
    EXPECT_TRUE(result);
    EXPECT_EQ(message, "Timesheet submitted.");
}
