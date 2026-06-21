#ifndef SERVER_SERVICES_INTERFACES_IEMAIL_SERVICE_H
#define SERVER_SERVICES_INTERFACES_IEMAIL_SERVICE_H

#include <string>

struct EmailMessage; // Forward declaration

class IEmailService
{
   public:
    virtual ~IEmailService() = default;
    virtual bool sendEmail(const EmailMessage& message, std::string& errorMessage) = 0;
};

#endif
