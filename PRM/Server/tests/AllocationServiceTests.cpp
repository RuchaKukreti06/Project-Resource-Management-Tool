#include <gtest/gtest.h>

#include <memory>
#include <unordered_map>

#include "MockAllocationRepository.h"
#include "MockEmployeeRepository.h"
#include "MockProjectRepository.h"
#include "repositories/IAllocationRepository.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/IProjectRepository.h"
#include "services/AllocationService.h"

class AllocationServiceTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        allocationRepo = std::make_shared<MockAllocationRepository>();
        employeeRepo = std::make_shared<MockEmployeeRepository>();
        projectRepo = std::make_shared<MockProjectRepository>();

        service = std::make_unique<AllocationService>(allocationRepo, employeeRepo, projectRepo);

        // put employee into the map so getEmployeeById(1) returns it
        Employee emp;
        emp.id = 1;
        emp.isActive = true;
        employeeRepo->employees[1] = emp;
        employeeRepo->employee = emp;  // keep for direct access if needed

        // set project directly — getProjectById returns project member
        projectRepo->project.id = 1;
        projectRepo->project.status = "ACTIVE";
    }

    std::shared_ptr<MockAllocationRepository> allocationRepo;
    std::shared_ptr<MockEmployeeRepository> employeeRepo;
    std::shared_ptr<MockProjectRepository> projectRepo;

    std::unique_ptr<AllocationService> service;
};

TEST_F(AllocationServiceTest, CreateAllocationSuccess)
{
    AllocationCreateRequest req;
    req.employeeId = 1;
    req.projectId = 1;
    req.utilizationPercentage = 100;
    req.fromDate = "2024-01-01";
    req.toDate = "2024-12-31";
    std::string message;

    EXPECT_TRUE(service->createAllocation(req, 10, message));

    EXPECT_EQ(message, "Allocation created.");
}

TEST_F(AllocationServiceTest, CreateAllocationInvalidPayload)
{
    AllocationCreateRequest req;

    std::string message;

    EXPECT_FALSE(service->createAllocation(req, 1, message));

    EXPECT_EQ(message, "Invalid allocation payload.");
}

TEST_F(AllocationServiceTest, CreateAllocationEmployeeNotFound)
{
    employeeRepo->employees.clear();

    AllocationCreateRequest req;
    req.employeeId = 1;
    req.projectId = 1;
    req.utilizationPercentage = 50;
    req.fromDate = "2026-01-01";
    req.toDate = "2026-02-01";

    std::string message;

    EXPECT_FALSE(service->createAllocation(req, 1, message));

    EXPECT_EQ(message, "Employee not active or not found.");
}

TEST_F(AllocationServiceTest, CreateAllocationProjectClosed)
{
    projectRepo->project.status = "COMPLETED";

    AllocationCreateRequest req;
    req.employeeId = 1;
    req.projectId = 1;
    req.utilizationPercentage = 50;
    req.fromDate = "2026-01-01";
    req.toDate = "2026-02-01";

    std::string message;

    EXPECT_FALSE(service->createAllocation(req, 1, message));

    EXPECT_EQ(message, "Project not found or not allocatable.");
}

TEST_F(AllocationServiceTest, CreateAllocationExceedsUtilization)
{
    allocationRepo->overlapUtilization = 60;

    AllocationCreateRequest req;
    req.employeeId = 1;
    req.projectId = 1;
    req.utilizationPercentage = 50;
    req.fromDate = "2026-01-01";
    req.toDate = "2026-02-01";

    std::string message;

    EXPECT_FALSE(service->createAllocation(req, 1, message));

    EXPECT_EQ(message, "Total utilization exceeds 100% in overlapping date range.");
}

TEST_F(AllocationServiceTest, CreateAllocationRepositoryFailure)
{
    allocationRepo->createResult = false;

    AllocationCreateRequest req;
    req.employeeId = 1;
    req.projectId = 1;
    req.utilizationPercentage = 50;
    req.fromDate = "2026-01-01";
    req.toDate = "2026-02-01";

    std::string message;

    EXPECT_FALSE(service->createAllocation(req, 1, message));

    EXPECT_EQ(message, "Failed to create allocation.");
}

TEST_F(AllocationServiceTest, EndAllocationSuccess)
{
    std::string message;

    EXPECT_TRUE(service->endAllocation({1, "2026-06-10"}, message));

    EXPECT_EQ(message, "Allocation ended.");
}

TEST_F(AllocationServiceTest, EndAllocationFailure)
{
    allocationRepo->endResult = false;

    std::string message;

    EXPECT_FALSE(service->endAllocation({1, "2026-06-10"}, message));

    EXPECT_EQ(message, "Failed to end allocation.");
}

TEST_F(AllocationServiceTest, CreateAllocationInactiveEmployee)
{
    employeeRepo->employees[1].isActive = false;

    AllocationCreateRequest req;
    req.employeeId = 1;
    req.projectId = 1;
    req.utilizationPercentage = 50;
    req.fromDate = "2026-01-01";
    req.toDate = "2026-02-01";

    std::string message;

    EXPECT_FALSE(service->createAllocation(req, 1, message));

    EXPECT_EQ(message, "Employee not active or not found.");
}
