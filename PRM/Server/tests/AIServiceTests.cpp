#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "services/AIService.h"
#include "mocks/MockEmployeeRepository.h"
#include "mocks/MockAllocationRepository.h"
#include "mocks/MockProjectRepository.h"
#include "mocks/MockTimesheetRepository.h"

class AIServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockEmployeeRepo = std::make_shared<MockEmployeeRepository>();
        mockAllocRepo = std::make_shared<MockAllocationRepository>();
        mockProjectRepo = std::make_shared<MockProjectRepository>();
        mockTimesheetRepo = std::make_shared<MockTimesheetRepository>();

        aiService = std::make_unique<AIService>(mockEmployeeRepo, mockAllocRepo, mockProjectRepo, mockTimesheetRepo);
    }

    std::shared_ptr<MockEmployeeRepository> mockEmployeeRepo;
    std::shared_ptr<MockAllocationRepository> mockAllocRepo;
    std::shared_ptr<MockProjectRepository> mockProjectRepo;
    std::shared_ptr<MockTimesheetRepository> mockTimesheetRepo;
    std::unique_ptr<AIService> aiService;
};

TEST_F(AIServiceTests, SkillMatch_NoApiKey_Fallback)
{
    Employee mockEmp; mockEmp.id = 1; mockEmp.user_id = 101; mockEmp.fullName = "Emp One"; mockEmp.isActive = true;
    mockEmployeeRepo->employees[1] = mockEmp;

    std::string result = aiService->skillMatch("Requirement", 40, "", "Mock");
    EXPECT_TRUE(result.find("Emp One") != std::string::npos);
    EXPECT_TRUE(result.find("AI key not configured") != std::string::npos);
}

TEST_F(AIServiceTests, RiskSummary_NoApiKey_Fallback)
{
    Project mockProj; mockProj.id = 1; mockProj.name = "Test Project"; mockProj.healthStatus = "GREEN";
    mockProjectRepo->project = mockProj;

    std::string result = aiService->riskSummary(1, "2024-01-01", "", "Mock");
    EXPECT_TRUE(result.find("Project Test Project is currently marked as GREEN") != std::string::npos);
}

TEST_F(AIServiceTests, TeamBuilder_NoApiKey_Fallback)
{
    std::string result = aiService->teamBuilder("Build team", "", "Mock");
    EXPECT_TRUE(result.find("No API key configured") != std::string::npos);
}

