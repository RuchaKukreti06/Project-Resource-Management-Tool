#include "services/AuthService.h"

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <openssl/evp.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

#include "ConfigLoader.h"
#include "UserRepository.h"

AuthService::AuthService(std::shared_ptr<IUserRepository> repository)
    : repository_(std::move(repository))
{
}

AuthService::~AuthService()
{
}

IUserRepository& AuthService::repository()
{
    if (!repository_)
    {
        repository_ = std::make_shared<UserRepository>(database::Database::instance());
    }
    return *repository_;
}

nlohmann::json AuthService::login(const std::string& username, const std::string& password)
{
    auto& repository = this->repository();
    nlohmann::json response;

    User user = repository.getUserByUsername(username);
    response["success"] = false;

    if (user.username.empty() || !verifyPassword(password, user.passwordHash))
    {
        response["message"] = "Invalid username or password.";
        return response;
    }

    if (user.status != "ACTIVE")
    {
        response["message"] = "Account is not active.";
        return response;
    }

    std::string token = generateToken(user);
    response["success"] = true;
    response["token"] = token;
    response["user"] = {{"id", user.id}, {"username", user.username}, {"role", user.role}};
    response["message"] = "Login successful.";
    return response;
}

nlohmann::json AuthService::registerUser(const std::string& username, const std::string& password)
{
    auto& repository = this->repository();
    nlohmann::json response;

    if (repository.getUserByUsername(username).username.empty())
    {
        User newUser;
        newUser.username = username;
        newUser.passwordHash = hashPassword(password);
        newUser.role = "user";
        newUser.status = "active";
        newUser.forcePasswordChange = false;

        if (repository.createUser(newUser))
        {
            response["success"] = true;
            response["message"] = "Registration successful.";
        }
        else
        {
            response["success"] = false;
            response["message"] = "Failed to create user.";
        }
    }
    else
    {
        response["success"] = false;
        response["message"] = "Username already exists.";
    }
    return response;
}

bool AuthService::isLoggedIn() const
{
    return token_.has_value() && validateToken(*token_);
}

void AuthService::setToken(const std::string& token)
{
    token_ = token;
}

bool AuthService::changePassword(int userId, const std::string& newPassword)
{
    auto& repository = this->repository();
    std::string newHash = hashPassword(newPassword);
    return repository.updatePassword(userId, newHash);
}

bool AuthService::validateToken(const std::string& token) const
{
    try
    {
        auto& config = utils::ConfigLoader::instance();
        auto secretKey = config.jwtSecret();
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{secretKey})
                            .with_issuer("PRM_Server");
        verifier.verify(decoded);
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Token validation failed: {}", e.what());
        return false;
    }
}

std::string AuthService::generateToken(const User& user)
{
    auto& config = utils::ConfigLoader::instance();
    auto secretKey = config.jwtSecret();
    auto expirationMinutes = config.jwtExpirationMinutes();

    auto token = jwt::create()
                     .set_issuer("PRM_Server")
                     .set_type("JWT")
                     .set_issued_at(std::chrono::system_clock::now())
                     .set_expires_at(std::chrono::system_clock::now() +
                                     std::chrono::minutes(expirationMinutes))
                     .set_payload_claim("id", jwt::claim(std::to_string(user.id)))
                     .set_payload_claim("username", jwt::claim(user.username))
                     .set_payload_claim("role", jwt::claim(user.role))
                     .sign(jwt::algorithm::hs256{secretKey});

    return token;
}

std::string AuthService::hashPassword(const std::string& password)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLength = 0;
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == nullptr)
    {
        throw std::runtime_error("Failed to create OpenSSL message digest context");
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(context, password.data(), password.size()) != 1 ||
        EVP_DigestFinal_ex(context, digest, &digestLength) != 1)
    {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to compute password hash");
    }
    EVP_MD_CTX_free(context);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < digestLength; ++i)
    {
        oss << std::setw(2) << static_cast<int>(digest[i]);
    }
    return oss.str();
}

bool AuthService::verifyPassword(const std::string& password, const std::string& hash)
{
    auto computedHash = hashPassword(password);
    return computedHash == hash;
}
