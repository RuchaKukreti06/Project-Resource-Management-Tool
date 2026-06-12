#ifndef APPLICATION_H
#define APPLICATION_H

#include "ApiClient.h"
#include "AuthSession.h"
#include "LoginScreen.h"

class Application
{
   private:
    ApiClient& apiClient;
    std::string baseUrl;

   public:
    Application(const std::string& baseUrl, ApiClient& apiClient);
    void run();
};

#endif
