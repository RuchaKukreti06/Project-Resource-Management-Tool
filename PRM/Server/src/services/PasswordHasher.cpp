#include "services/PasswordHasher.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <argon2.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include "exceptions/Exceptions.h"

// Argon2id parameters
constexpr uint32_t ARGON2_T_COST = 2;          // Iterations
constexpr uint32_t ARGON2_M_COST = 1 << 16;    // 64 MiB
constexpr uint32_t ARGON2_PARALLELISM = 1;     // Threads
constexpr size_t ARGON2_SALT_LEN = 16;         // 16 bytes salt
constexpr size_t ARGON2_HASH_LEN = 32;         // 32 bytes hash output

// Helper to compute legacy SHA-256 hash
static std::string computeLegacySHA256(const std::string& password)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int  digestLength = 0;
    EVP_MD_CTX*   context      = EVP_MD_CTX_new();
    if (context == nullptr)
    {
        throw exceptions::AppException("Failed to create OpenSSL message digest context");
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(context, password.data(), password.size()) != 1 ||
        EVP_DigestFinal_ex(context, digest, &digestLength) != 1)
    {
        EVP_MD_CTX_free(context);
        throw exceptions::AppException("Failed to compute legacy password hash");
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

// Strict validation helper for legacy SHA-256 hashes
static bool isValidSha256Hash(const std::string& hash)
{
    if (hash.length() != 64)
    {
        return false;
    }
    return hash.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
}

std::string PasswordHasher::hashPassword(const std::string& password) const
{
    uint8_t salt[ARGON2_SALT_LEN];
    if (RAND_bytes(salt, sizeof(salt)) != 1)
    {
        throw exceptions::AppException("Failed to generate secure random salt");
    }

    size_t encoded_len = argon2_encodedlen(
        ARGON2_T_COST, ARGON2_M_COST, ARGON2_PARALLELISM, ARGON2_SALT_LEN, ARGON2_HASH_LEN, Argon2_id);
        
    std::vector<char> buffer(encoded_len, '\0');

    int result = argon2id_hash_encoded(
        ARGON2_T_COST, ARGON2_M_COST, ARGON2_PARALLELISM,
        password.data(), password.length(),
        salt, ARGON2_SALT_LEN, ARGON2_HASH_LEN,
        buffer.data(), encoded_len);

    if (result != ARGON2_OK)
    {
        throw exceptions::AppException("Failed to compute Argon2id password hash");
    }
    
    return std::string(buffer.data());
}

bool PasswordHasher::verifyPassword(const std::string& password, const std::string& hash) const
{
    if (hash.empty()) return false;

    // Check if it's an Argon2id hash
    if (hash.rfind("$argon2id$", 0) == 0)
    {
        int result = argon2id_verify(hash.c_str(), password.data(), password.length());
        return result == ARGON2_OK;
    }

    // Fallback: Check if it's a valid legacy SHA-256 hash
    if (isValidSha256Hash(hash))
    {
        return computeLegacySHA256(password) == hash;
    }

    // Unknown hash format safely rejected
    return false;
}
