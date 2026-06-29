#pragma once

#include <stdexcept>
#include <string>

namespace exceptions
{

class AppException : public std::runtime_error
{
   public:
    explicit AppException(const std::string& message) : std::runtime_error(message)
    {
    }
};

class ValidationException : public AppException
{
   public:
    explicit ValidationException(const std::string& message) : AppException(message)
    {
    }
};

class ConflictException : public AppException
{
   public:
    explicit ConflictException(const std::string& message) : AppException(message)
    {
    }
};

class NotFoundException : public AppException
{
   public:
    explicit NotFoundException(const std::string& message) : AppException(message)
    {
    }
};

class AuthenticationException : public AppException
{
   public:
    explicit AuthenticationException(const std::string& message) : AppException(message)
    {
    }
};

class AuthorizationException : public AppException
{
   public:
    explicit AuthorizationException(const std::string& message) : AppException(message)
    {
    }
};

class DatabaseException : public AppException
{
   public:
    explicit DatabaseException(const std::string& message) : AppException(message)
    {
    }
};

}  // namespace exceptions