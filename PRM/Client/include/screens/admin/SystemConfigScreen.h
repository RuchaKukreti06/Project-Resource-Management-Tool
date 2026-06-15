#ifndef SYSTEM_CONFIG_SCREEN_H
#define SYSTEM_CONFIG_SCREEN_H

#include <nlohmann/json.hpp>
#include <string>
#include "Screen.h"

class SystemConfigScreen : public Screen
{
   public:
    SystemConfigScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    bool        keepRunning_       = true;
    std::string llmProvider_       = "Google Gemini";
    std::string llmApiKey_;
    int         schedulerInterval_ = 4;
    int         maxWeeklyHours_    = 40;

    void loadConfig(ApiClient& apiClient);
    bool saveConfig(ApiClient& apiClient, const nlohmann::json& patch);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
