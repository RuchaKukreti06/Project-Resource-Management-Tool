#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "services/EmployeeService.h"
#include "mocks/MockEmployeeRepository.h"
#include "mocks/MockUserRepository.h"
#include "mocks/MockAllocationRepository.h"

class EmployeeServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockEmployeeRepo = std::make_shared<MockEmployeeRepository>();
        mockUserRepo = std::make_shared<MockUserRepository>();
        mockAllocRepo = std::make_shared<MockAllocationRepository>();
        employeeService = std::make_unique<EmployeeService>(mockEmployeeRepo, mockUserRepo, mockAllocRepo);
    }

    std::shared_ptr<MockEmployeeRepository> mockEmployeeRepo;
    std::shared_ptr<MockUserRepository> mockUserRepo;
    std::shared_ptr<MockAllocationRepository> mockAllocRepo;
    std::unique_ptr<EmployeeService> employeeService;
};

TEST_F(EmployeeServiceTests, GetTeamEmployees_ReturnsEmployees)
{
    Employee e1; e1.id = 1; e1.user_id = 100; e1.fullName = "Emp One"; e1.email = "emp1@test.com"; e1.department = "IT"; e1.designation = "Dev"; e1.status = "ACTIVE"; e1.isActive = true;
    Employee e2; e2.id = 2; e2.user_id = 100; e2.fullName = "Emp Two"; e2.email = "emp2@test.com"; e2.department = "IT"; e2.designation = "QA"; e2.status = "ON_BENCH"; e2.isActive = true;
    mockEmployeeRepo->employees[1] = e1;
    mockEmployeeRepo->employees[2] = e2;

    auto emps = employeeService->getTeamEmployees(100);
    EXPECT_EQ(emps.size(), 2);
}

TEST_F(EmployeeServiceTests, AddSkill_Success)
{
    std::string message;
    bool result = employeeService->addSkill(1, "C++", "BACKEND", "INTERMEDIATE", message);
    EXPECT_TRUE(result);
    EXPECT_EQ(message, "Skill added.");
}

TEST_F(EmployeeServiceTests, RemoveSkill_Success)
{
    std::string message;
    bool result = employeeService->removeSkill(1, 10, message);
    EXPECT_TRUE(result);
    EXPECT_EQ(message, "Skill removed.");
}

TEST_F(EmployeeServiceTests, GetEmployeeById_ValidId_ReturnsEmployee)
{
    Employee mockEmp; mockEmp.id = 1; mockEmp.user_id = 101; mockEmp.fullName = "Emp One"; mockEmp.email = "emp1@test.com"; mockEmp.department = "IT"; mockEmp.designation = "Dev"; mockEmp.status = "ACTIVE"; mockEmp.isActive = true;
    mockEmployeeRepo->employees[1] = mockEmp;

    auto empOpt = employeeService->getEmployeeById(1);
    EXPECT_TRUE(empOpt.has_value());
    EXPECT_EQ(empOpt->fullName, "Emp One");
}

