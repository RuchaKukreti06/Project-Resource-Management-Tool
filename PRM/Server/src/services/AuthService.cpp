#include "AuthService.h"
#include "repositories/UserRepository.h"
#include <bcrypt/BCrypt.hpp>

AuthService::AuthService() {}

AuthService::~AuthService() {}

nlohmann::json AuthService::login(const std::string& username, const std::string& password) {
    UserRepository userRepository;
    nlohmann::json response;

    User user = userRepository.getUserByUsername(username);
    response["success"] = false;

    if (user.username.empty()) {
        response["message"] = "Invalid username or password.";
    }
    if (!verifyPassword(password, user.passwordHash)) {
        response["message"] = "Invalid username or password.";
    }
    else if (user.status != "active") {
        response["message"] = "Account is not active.";
    }
    else{
        std::string token = generateToken(user);
        response["success"] = true;
        response["token"] = token;
        response["user"] = {
            {"id", user.id},
            {"username", user.username},
            {"role", user.role}
        };
        response["message"] = "Login successful.";
    }
    return response;
}

nlohmann::json AuthService::registerUser(const std::string& username, const std::string& password) {
    UserRepository userRepository;
    nlohmann::json response;
    
    if (userRepository.getUserByUsername(username).username.empty()) {
        User newUser;
        newUser.username = username;
        newUser.passwordHash = hashPassword(password);
        newUser.role = "user";
        newUser.status = "active";
        newUser.forcePasswordChange = false;

        if (userRepository.createUser(newUser)) {
            response["success"] = true;
            response["message"] = "Registration successful.";
        } else {
            response["success"] = false;
            response["message"] = "Failed to create user.";
        }
    } else {
        response["success"] = false;
        response["message"] = "Username already exists.";
    }
    return response;
}

bool AuthService::isLoggedIn() const {
    if (auto token = Utils:: getTokenFromHeader()) {
        return validateToken(*token);
    }
    return false;
}

bool AuthService::changePassword(int userId, const std::string& newPassword) {
    UserRepository userRepository;
    std::string newHash = hashPassword(newPassword);
    return userRepository.updatePassword(userId, newHash);
}

bool AuthService::validateToken(const std::string& token) {
    auto config = Utils:: ConfigLoader :: getConfig();
    auto secretKey = config["jwt_secret"].get<std::string>();
    bool isValid = false;
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secretKey})
            .with_issuer("PRM_Server");
        verifier.verify(decoded);
        isValid = true;
    } catch (const std::exception& e) {
        spdlog::error("Token validation failed: {}", e.what());
    }
    return isValid;
}

std::string AuthService::hashPassword(const std::string& password) {
    bcrypt::bcrypt_hash passwordHash = bcrypt::bcrypt_hash(password);
    return passwordHash.hash;
}

bool AuthService::verifyPassword(const std::string& password, const std::string& hash) {
    return bcrypt::bcrypt_checkpw(password.c_str(), hash.c_str()) == 0;
}
