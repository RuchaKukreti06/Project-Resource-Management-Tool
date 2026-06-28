#include "app/Application.h"
#include "app/Router.h"
#include "app/AppServices.h"
#include "api/ISessionStore.h"

#include <iostream>

Application::Application(IApiClient& apiClient, api::ISessionStore& sessionStore)
{
    services_ = std::make_unique<AppServices>(apiClient, sessionStore);
    router_ = std::make_unique<Router>(*services_);
}

Application::~Application() = default;

void Application::run()
{
    try 
    {
        router_->start();
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n[FATAL ERROR] " << e.what() << "\nTerminating application.\n";
    }
}
