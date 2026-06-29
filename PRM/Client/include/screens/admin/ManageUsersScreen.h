#ifndef MANAGE_USERS_SCREEN_H
#define MANAGE_USERS_SCREEN_H

#include "screens/Screen.h"
#include "dto/UserDTO.h"
#include <optional>
#include <string>
#include "admin/adminConstants.h"

class UserClientService;
namespace api { class ISessionStore; }

class ManageUsersScreen : public Screen
{
   public:
    ManageUsersScreen(UserClientService& userService, api::ISessionStore& sessionStore);
    void show() override;
    void displayMenu() override;
    void handleInput() override;
    void createUser();
    void viewUsers();
    void resetUserPassword();
    void deactivateUser();

   private:
    void displayUsers(const std::vector<UserDTO>& users);
    std::optional<std::string> promptForTargetUserId(const std::string& prompt);
    bool promptForBasicUserInfo(CreateUserRequest& req);
    void promptForCredentials(CreateUserRequest& req);
    bool promptForRoleAndDepartmentDetails(CreateUserRequest& req);
    std::optional<CreateUserRequest> promptForUserDetails();
    std::string promptForValidEmail();
    std::string promptForValidPassword();
    std::optional<std::string> promptForRole();

    void handleReactivation();

    UserClientService& userService_;
    api::ISessionStore& sessionStore_;
    bool keepRunning_ = true;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
