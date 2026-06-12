#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>

#include "ApiClient.h"
#include "ScreenDecorator.h"
#include "ScreenInput.h"
#include "ScreenOptions.h"
#include "ScreenUtils.h"

class Screen
{
   protected:
    virtual ScreenDecorator decorator() const = 0;
    void clearScreen() const;
    void printDivider() const;
    void showError(const std::string& msg);
    void showSuccess(const std::string& msg);
    void showInfo(const std::string& msg);
    void showWarning(const std::string& msg);

   public:
    virtual ~Screen() = default;
    virtual void show(ApiClient& apiClient) = 0;
    virtual void displayMenu() = 0;
    virtual void handleInput(ApiClient& apiClient) = 0;
};

#endif