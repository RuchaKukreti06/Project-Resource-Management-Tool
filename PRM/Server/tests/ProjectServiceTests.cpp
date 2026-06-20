#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "services/ProjectService.h"
#include "mocks/MockProjectRepository.h"
#include "mocks/MockUserRepository.h"

class ProjectServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockProjectRepo = std::make_shared<MockProjectRepository>();
        mockUserRepo = std::make_shared<MockUserRepository>();
        projectService = std::make_unique<ProjectService>(mockProjectRepo, mockUserRepo);
    }

    std::shared_ptr<MockProjectRepository> mockProjectRepo;
    std::shared_ptr<MockUserRepository> mockUserRepo;
    std::unique_ptr<ProjectService> projectService;
};

TEST_F(ProjectServiceTests, CreateProject_ValidManager_Success)
{
    User mockManager; mockManager.id = 1; mockManager.username = "manager"; mockManager.role = "MANAGER"; mockManager.status = "ACTIVE"; mockManager.isActive = true;
    mockUserRepo->createUser(mockManager);

    ProjectCreateRequest req;
    req.name = "Project Alpha";
    req.managerId = 1;
    req.status = "PLANNED";
    std::string message;
    bool result = projectService->createProject(req, message);
    EXPECT_TRUE(result);
    EXPECT_EQ(message, "Project created.");
}

TEST_F(ProjectServiceTests, CreateProject_InvalidManager_Fails)
{
    ProjectCreateRequest req;
    req.managerId = 2; // User doesn't exist
    req.name = "Test Project";

    std::string message;
    bool result = projectService->createProject(req, message);
    EXPECT_FALSE(result);
    EXPECT_EQ(message, "Invalid manager id.");
}

TEST_F(ProjectServiceTests, GetAllProjects_ReturnsList)
{
    Project mockProj; mockProj.id = 1; mockProj.name = "Project 1"; mockProj.description = "Desc"; mockProj.startDate = "2024-01-01"; mockProj.endDate = "2024-12-31"; mockProj.managerId = 100; mockProj.status = "ACTIVE"; mockProj.healthStatus = "GREEN";
    std::vector<Project> mockProjects = { mockProj };
    mockProjectRepo->projects = mockProjects;

    auto projects = projectService->getAllProjects();
    EXPECT_EQ(projects.size(), 1);
    EXPECT_EQ(projects[0].name, "Project 1");
}

TEST_F(ProjectServiceTests, AddMilestone_Success)
{
    AddMilestoneRequest m;
    m.projectId = 1;
    m.title = "Milestone 1";
    
    std::string message;
    bool result = projectService->addMilestone(m, message);
    EXPECT_TRUE(result);
    EXPECT_EQ(message, "Milestone added.");
}

