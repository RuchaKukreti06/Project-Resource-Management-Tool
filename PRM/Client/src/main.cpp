#include "ApiClient.h"
#include "Application.h"
#include "AuthSession.h"

#include <windows.h>
#include "utils/Constants.h"

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    ApiClient apiClient(constants::BASE_URL);
    api::AuthSession sessionStore;
    Application application(apiClient, sessionStore);
    application.run();

    return 0;
}