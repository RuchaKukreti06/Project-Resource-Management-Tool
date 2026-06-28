#ifndef SYSTEM_CONFIG_SCREEN_H
#define SYSTEM_CONFIG_SCREEN_H

#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include "admin/adminConstants.h"
#include "Screen.h"

class ConfigClientService;
struct SystemConfigDTO;

class SystemConfigScreen : public Screen
{
   public:
    SystemConfigScreen(ConfigClientService& configService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    ConfigClientService& configService_;
    bool        keepRunning_       = true;
    std::string llmProvider_       = "Google Gemini";
    std::string llmApiKey_;
    int         schedulerInterval_ = 4;
    int         maxWeeklyHours_    = 40;
    bool        smtpEnabled_       = false;
    std::string smtpHost_;
    int         smtpPort_          = 587;
    std::string smtpUsername_;
    std::string smtpPasswordMasked_;
    std::string smtpFromEmail_;
    std::string smtpFromName_;
    bool        smtpUseTls_        = true;

    void loadConfig();
    void applyConfig(const SystemConfigDTO& config);
    bool saveConfig(const nlohmann::json& patch);
    bool sendTestEmail(const std::string& toEmail);

    void handleUpdateLlmKey();
    void handleChangeLlmProvider();
    void handleUpdateSchedulerInterval();
    void handleUpdateWeeklyHours();
    void handleToggleSmtp();
    void handleUpdateSmtpServer();
    void handleUpdateSmtpCredentials();
    void handleUpdateSmtpSender();
    void handleToggleSmtpTls();
    void handleSendTestEmail();

    // New Helpers
    void displayCurrentSettings();
    void displayConfigOptions();
    std::optional<int> promptForIntInRange(const std::string& prompt, int minVal, int maxVal);
    void saveConfigAndNotify(const nlohmann::json& patch, const std::string& successMsg);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
