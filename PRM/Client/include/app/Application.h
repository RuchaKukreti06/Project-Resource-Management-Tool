#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include <string>

class Router;
struct AppServices;
namespace api { class ISessionStore; }
class IApiClient;

class Application
{
   private:
    std::string baseUrl;
    std::unique_ptr<AppServices> services_;
    std::unique_ptr<Router> router_;

   public:
    Application(const std::string& baseUrl, IApiClient& apiClient);
    ~Application();
    void run();
};

#endif
