# ifndef SERVER_AUTH_SERVICE_H
# define SERVER_AUTH_SERVICE_H

#include "User.h"
#include "UserRepository.h"
#include <nlohmann/json.hpp>

class AuthService{
public:
    AuthService();
    ~AuthService();
    nlohmann::json login(const std::string& username, const std::string& password);
    nlohmann::json registerUser(const std::string& username, const std::string& password);
    bool isLoggedIn() const;
    bool changePassword(int userId, const std::string& newPassword);
    bool validateToken(const std::string& token);

private:
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string generateToken(const User& user);
};

#endif