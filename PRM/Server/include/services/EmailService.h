#ifndef SERVER_SERVICES_EMAIL_SERVICE_H
#define SERVER_SERVICES_EMAIL_SERVICE_H

#include <cstddef>
#include <memory>
#include <string>

#include "repositories/ISystemConfigRepository.h"

struct EmailMessage
{
    std::string to;
    std::string subject;
    std::string body;
};

class EmailService
{
   public:
    explicit EmailService(std::shared_ptr<ISystemConfigRepository> configRepository);
    ~EmailService();

    bool sendEmail(const EmailMessage& message, std::string& errorMessage);

   private:
    static size_t payloadSource(char* ptr, size_t size, size_t nmemb, void* userData);

    std::shared_ptr<ISystemConfigRepository> configRepository_;
};

#endif