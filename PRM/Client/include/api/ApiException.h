#ifndef API_EXCEPTION_H
#define API_EXCEPTION_H

#include <stdexcept>
#include <string>

class ApiException : public std::runtime_error
{
public:
    explicit ApiException(const std::string& message, int statusCode = 0)
        : std::runtime_error(message), statusCode_(statusCode) {}

    int getStatusCode() const { return statusCode_; }

private:
    int statusCode_;
};

class AuthenticationException : public ApiException
{
public:
    explicit AuthenticationException(const std::string& message)
        : ApiException(message, 401) {}
};

class AuthorizationException : public ApiException
{
public:
    explicit AuthorizationException(const std::string& message)
        : ApiException(message, 403) {}
};

class ValidationException : public ApiException
{
public:
    explicit ValidationException(const std::string& message)
        : ApiException(message, 400) {}
};

class NotFoundException : public ApiException
{
public:
    explicit NotFoundException(const std::string& message)
        : ApiException(message, 404) {}
};

class ServerException : public ApiException
{
public:
    explicit ServerException(const std::string& message, int statusCode = 500)
        : ApiException(message, statusCode) {}
};

class NetworkException : public ApiException
{
public:
    explicit NetworkException(const std::string& message)
        : ApiException(message, 0) {}
};

#endif // API_EXCEPTION_H
