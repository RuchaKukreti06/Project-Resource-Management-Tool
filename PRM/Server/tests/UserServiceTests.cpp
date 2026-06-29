#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "services/PasswordHasher.h"
#include "services/UserService.h"
#include "mocks/MockUserRepository.h"

class UserServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockRepo = std::make_shared<MockUserRepository>();
        auto hasher = std::make_shared<PasswordHasher>();
        userService = std::make_unique<UserService>(mockRepo, hasher);
    }

    std::shared_ptr<MockUserRepository> mockRepo;
    std::unique_ptr<UserService> userService;
};

TEST_F(UserServiceTests, GetAllUsers_ReturnsList)
{
    User u1; u1.id = 1; u1.username = "testuser"; u1.role = "ADMIN";
    User u2; u2.id = 2; u2.username = "test2"; u2.role = "EMPLOYEE";
    mockRepo->createUser(u1);
    mockRepo->createUser(u2);

    auto users = userService->getAllUsers();
    EXPECT_EQ(users.size(), 2);
}

TEST_F(UserServiceTests, GetUserById_ValidId_ReturnsUser)
{
    User u1; u1.id = 1; u1.username = "testuser"; u1.role = "ADMIN";
    mockRepo->createUser(u1);

    auto userOpt = userService->getUserById(1);
    EXPECT_TRUE(userOpt.has_value());
    EXPECT_EQ(userOpt->username, "testuser");
}

TEST_F(UserServiceTests, CreateUser_Success)
{
    UserCreateRequest req;
    req.username = "newuser";
    req.password = "Valid!234";
    req.role = "EMPLOYEE";
    req.email = "email@test.com";
    req.fullName = "New User";
    req.department = "Dept";
    req.designation = "Desig";
    bool result = userService->createUser(req);
    EXPECT_TRUE(result);
}

TEST_F(UserServiceTests, DeactivateUser_Success)
{
    User u1; u1.id = 1; u1.username = "testuser"; u1.status = "ACTIVE"; u1.isActive = true;
    mockRepo->createUser(u1);

    bool result = userService->deactivateUser(1);
    EXPECT_TRUE(result);
    EXPECT_EQ(mockRepo->getUserById(1).status, "INACTIVE");
}

