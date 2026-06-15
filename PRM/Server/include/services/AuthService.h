#ifndef SERVER_AUTH_SERVICE_H
#define SERVER_AUTH_SERVICE_H

#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "models/User.h"
#include "repositories/IUserRepository.h"

class AuthService
{
   public:
    explicit AuthService(std::shared_ptr<IUserRepository> repository = nullptr);
    ~AuthService();

    nlohmann::json login(const std::string& username, const std::string& password);
    nlohmann::json registerUser(const std::string& username, const std::string& password,
                                const std::string& email, const std::string& fullName);
    bool isLoggedIn() const;
    void setToken(const std::string& token);
    bool changePassword(int userId, const std::string& newPassword);
    bool validateToken(const std::string& token) const;

   private:
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string generateToken(const User& user);
    IUserRepository& repository();

    std::shared_ptr<IUserRepository> repository_;
    std::optional<std::string> token_;
};

#endif