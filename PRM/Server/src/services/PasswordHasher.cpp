#include "services/PasswordHasher.h"

#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string PasswordHasher::hashPassword(const std::string& password) const
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int  digestLength = 0;
    EVP_MD_CTX*   context      = EVP_MD_CTX_new();
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

bool PasswordHasher::verifyPassword(const std::string& password, const std::string& hash) const
{
    return hashPassword(password) == hash;
}
