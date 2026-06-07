#include "Application.h"

int main()
{
    Application app;
    if (!app.run())
    {
        return 1;
    }
    return 0;
}