#include "app/Application.h"
#include "app/Router.h"
#include "app/AppServices.h"
#include "AuthSession.h"

Application::Application(const std::string& baseUrl, IApiClient& apiClient)
    : baseUrl(baseUrl)
{
    services_ = std::make_unique<AppServices>(apiClient, api::AuthSession::instance());
    router_ = std::make_unique<Router>(*services_);
}

Application::~Application() = default;

void Application::run()
{
    router_->start();
}
