#include "ApiClient.h"
#include "Application.h"
#include "AuthSession.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    const std::string baseUrl = "http://localhost:8080";
    ApiClient apiClient(baseUrl);
    Application app(baseUrl, apiClient);
    app.run();

    return 0;
}