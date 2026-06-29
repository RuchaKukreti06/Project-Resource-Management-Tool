#include "admin/SystemConfigScreen.h"
#include "dto/SystemConfigDTO.h"
#include "dto/ApiResponse.h"
#include "api/ApiException.h"
#include "utils/ConsoleInput.h"
#include "services/ConfigClientService.h"
#include "screens/ScreenUtils.h"
#include <iostream>

using namespace AdminConstants;
using namespace AdminConstants::Config;

SystemConfigScreen::SystemConfigScreen(ConfigClientService& configService)
    : configService_(configService)
{
}

void SystemConfigScreen::show()
{
    loadConfig();

    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput();
    }
}

void SystemConfigScreen::applyConfig(const SystemConfigDTO& config)
{
    llmProvider_       = config.llmProvider;
    llmApiKey_         = config.llmApiKey;
    schedulerInterval_ = config.schedulerIntervalHours;
    maxWeeklyHours_    = config.maxWeeklyHours;
    smtpEnabled_       = config.smtpEnabled;
    smtpHost_          = config.smtpHost;
    smtpPort_          = config.smtpPort;
    smtpUsername_      = config.smtpUsername;
    smtpPasswordMasked_ = config.smtpPassword;
    smtpFromEmail_     = config.smtpFromEmail;
    smtpFromName_      = config.smtpFromName;
    smtpUseTls_        = config.smtpUseTls;
}

void SystemConfigScreen::loadConfig()
{
    try
    {
        auto configResponse = configService_.getConfig();
        if (configResponse.success && configResponse.data.has_value())
        {
            applyConfig(configResponse.data.value());
        }
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

bool SystemConfigScreen::sendTestEmail(const std::string& toEmail)
{
    try
    {
        auto testEmailResponse = configService_.sendTestEmail(toEmail, "PRM SMTP Test", "SMTP configuration test succeeded.");
        if (!testEmailResponse.success)
        {
            showError(testEmailResponse.message);
            return false;
        }
        return true;
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        return false;
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
        return false;
    }
}

bool SystemConfigScreen::saveConfig(const nlohmann::json& configPatch)
{
    try
    {
        auto updateResponse = configService_.updateConfig(configPatch);
        if (!updateResponse.success)
        {
            showError(updateResponse.message);
            return false;
        }
        loadConfig();  // Refresh local values from server
        return true;
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        return false;
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
        return false;
    }
}

void SystemConfigScreen::displayCurrentSettings()
{
    std::cout << "\nCurrent Settings:\n";
    std::cout << "  LLM Provider        :  " << llmProvider_ << "\n";
    std::cout << "  LLM API Key         :  "
              << (llmApiKey_.empty() ? "(not set)" : std::string(llmApiKey_.size(), '*')) << "\n";
    std::cout << "  Scheduler Interval  :  " << schedulerInterval_ << " hours\n";
    std::cout << "  Max Weekly Hours    :  " << maxWeeklyHours_ << "\n";
    std::cout << "  SMTP Enabled        :  " << (smtpEnabled_ ? "Yes" : "No") << "\n";
    std::cout << "  SMTP Host           :  " << (smtpHost_.empty() ? "(not set)" : smtpHost_) << "\n";
    std::cout << "  SMTP Port           :  " << smtpPort_ << "\n";
    std::cout << "  SMTP Username       :  "
              << (smtpUsername_.empty() ? "(not set)" : smtpUsername_) << "\n";
    std::cout << "  SMTP Password       :  "
              << (smtpPasswordMasked_.empty() ? "(not set)" : smtpPasswordMasked_) << "\n";
    std::cout << "  SMTP From Email     :  "
              << (smtpFromEmail_.empty() ? "(not set)" : smtpFromEmail_) << "\n";
    std::cout << "  SMTP From Name      :  "
              << (smtpFromName_.empty() ? "(not set)" : smtpFromName_) << "\n";
    std::cout << "  SMTP TLS            :  " << (smtpUseTls_ ? "Enabled" : "Disabled") << "\n";
    ScreenUtils::printDivider();
}

void SystemConfigScreen::displayConfigOptions()
{
    std::cout << OPT_LLM_KEY << ". Update LLM API Key\n";
    std::cout << OPT_LLM_PROV << ". Change LLM Provider (Gemini / Groq / Gemma Remote)\n";
    std::cout << OPT_SCHED << ". Update Scheduler Interval\n";
    std::cout << OPT_HOURS << ". Update Max Weekly Hours\n";
    std::cout << OPT_SMTP_EN << ". Enable/Disable SMTP\n";
    std::cout << OPT_SMTP_SRV << ". Update SMTP Server (Host + Port)\n";
    std::cout << OPT_SMTP_CRED << ". Update SMTP Credentials\n";
    std::cout << OPT_SMTP_SND << ". Update SMTP Sender\n";
    std::cout << OPT_SMTP_TLS << ". Toggle SMTP TLS\n";
    std::cout << OPT_TEST_EMAIL << ". Send Test Email\n";
    std::cout << OPT_BACK << ". Back\n";
}

void SystemConfigScreen::displayMenu()
{
    decorator().render();
    displayCurrentSettings();
    displayConfigOptions();
}

void SystemConfigScreen::handleInput()
{
    std::string choice = ConsoleInput::readLine("Enter option");

    if (choice == OPT_LLM_KEY)
    {
        handleUpdateLlmKey();
    }
    else if (choice == OPT_LLM_PROV)
    {
        handleChangeLlmProvider();
    }
    else if (choice == OPT_SCHED)
    {
        handleUpdateSchedulerInterval();
    }
    else if (choice == OPT_HOURS)
    {
        handleUpdateWeeklyHours();
    }
    else if (choice == OPT_SMTP_EN)
    {
        handleToggleSmtp();
    }
    else if (choice == OPT_SMTP_SRV)
    {
        handleUpdateSmtpServer();
    }
    else if (choice == OPT_SMTP_CRED)
    {
        handleUpdateSmtpCredentials();
    }
    else if (choice == OPT_SMTP_SND)
    {
        handleUpdateSmtpSender();
    }
    else if (choice == OPT_SMTP_TLS)
    {
        handleToggleSmtpTls();
    }
    else if (choice == OPT_TEST_EMAIL)
    {
        handleSendTestEmail();
    }
    else if (choice == OPT_BACK || ScreenUtils::equalsIgnoreCase(choice, "B"))
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1-11.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

void SystemConfigScreen::saveConfigAndNotify(const nlohmann::json& configPatch, const std::string& successMsg)
{
    if (saveConfig(configPatch))
    {
        showSuccess(successMsg);
    }
    ConsoleInput::waitForEnter("Press Enter to continue\n");
}

std::optional<int> SystemConfigScreen::promptForIntInRange(const std::string& prompt, int minVal, int maxVal)
{
    std::string input = ConsoleInput::readLine(prompt);
    auto parsedValOpt = ScreenUtils::safeParseInt(input);
    
    if (!parsedValOpt)
    {
        showError("Invalid number format");
        return std::nullopt;
    }
    
    int parsedValue = parsedValOpt.value();
    if (parsedValue < minVal || parsedValue > maxVal)
    {
        showError("Value must be between " + std::to_string(minVal) + " and " + std::to_string(maxVal) + ".");
        return std::nullopt;
    }
    
    return parsedValue;
}

void SystemConfigScreen::handleUpdateLlmKey()
{
    std::string key = ConsoleInput::readLine("Enter new LLM API Key");
    if (key.empty())
    {
        showError("API Key cannot be empty.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    saveConfigAndNotify({{"llm_api_key", key}}, "System Settings updated successfully. ✓");
}

void SystemConfigScreen::handleChangeLlmProvider()
{
    std::cout << "Select Provider:\n";
    std::cout << "1. Google Gemini\n";
    std::cout << "2. Groq\n";
    std::cout << "3. Gemma (Remote)\n";
    std::string prov = ConsoleInput::readLine("Choice");
    std::string providerName;
    if      (prov == "1") providerName = "Google Gemini";
    else if (prov == "2") providerName = "Groq";
    else if (prov == "3") providerName = "Gemma (Remote)";
    else
    {
        showError("Invalid choice.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    saveConfigAndNotify({{"llm_provider", providerName}}, "LLM Provider updated to " + providerName + ".");
}

void SystemConfigScreen::handleUpdateSchedulerInterval()
{
    auto schedulerIntervalHoursOpt = promptForIntInRange("Enter Scheduler Interval in hours (1-168)", MIN_HOURS_PER_WEEK, MAX_HOURS_PER_WEEK);
    if (!schedulerIntervalHoursOpt)
    {
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    saveConfigAndNotify({{"scheduler_interval", schedulerIntervalHoursOpt.value()}}, "Scheduler Interval updated to " + std::to_string(schedulerIntervalHoursOpt.value()) + " hours.");
}

void SystemConfigScreen::handleUpdateWeeklyHours()
{
    auto maxWeeklyHoursOpt = promptForIntInRange("Enter Max Weekly Hours (1-168)", MIN_HOURS_PER_WEEK, MAX_HOURS_PER_WEEK);
    if (!maxWeeklyHoursOpt)
    {
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    saveConfigAndNotify({{"max_weekly_hours", maxWeeklyHoursOpt.value()}}, "Max Weekly Hours updated to " + std::to_string(maxWeeklyHoursOpt.value()) + ".");
}

void SystemConfigScreen::handleToggleSmtp()
{
    saveConfigAndNotify({{"smtp_enabled", !smtpEnabled_}}, std::string("SMTP ") + (!smtpEnabled_ ? "enabled." : "disabled."));
}

void SystemConfigScreen::handleUpdateSmtpServer()
{
    std::string host = ConsoleInput::readLine("Enter SMTP host");
    if (host.empty())
    {
        showError("SMTP host cannot be empty.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    auto portOpt = promptForIntInRange("Enter SMTP port", MIN_SMTP_PORT, MAX_SMTP_PORT);
    if (!portOpt)
    {
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    saveConfigAndNotify({{"smtp_host", host}, {"smtp_port", portOpt.value()}}, "SMTP server updated.");
}

void SystemConfigScreen::handleUpdateSmtpCredentials()
{
    std::string username = ConsoleInput::readLine("Enter SMTP username");
    std::string password = ScreenUtils::readPassword("Enter SMTP password");
    if (password.empty())
    {
        showError("SMTP password cannot be empty.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    saveConfigAndNotify({{"smtp_username", username}, {"smtp_password", password}}, "SMTP credentials updated.");
}

void SystemConfigScreen::handleUpdateSmtpSender()
{
    std::string fromEmail = ConsoleInput::readLine("Enter SMTP from email");
    if (fromEmail.empty() || !ScreenUtils::isValidEmail(fromEmail))
    {
        showError("Valid SMTP from email is required.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    std::string fromName = ConsoleInput::readLine("Enter SMTP from name (optional)");
    
    saveConfigAndNotify({{"smtp_from_email", fromEmail}, {"smtp_from_name", fromName}}, "SMTP sender updated.");
}

void SystemConfigScreen::handleToggleSmtpTls()
{
    saveConfigAndNotify({{"smtp_use_tls", !smtpUseTls_}}, std::string("SMTP TLS ") + (!smtpUseTls_ ? "enabled." : "disabled."));
}

void SystemConfigScreen::handleSendTestEmail()
{
    std::string toEmail = ConsoleInput::readLine("Enter recipient email");
    if (toEmail.empty() || !ScreenUtils::isValidEmail(toEmail))
    {
        showError("Valid recipient email is required.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    if (sendTestEmail(toEmail))
    {
        showSuccess("Test email sent successfully. ✓");
    }
    ConsoleInput::waitForEnter("Press Enter to continue\n");
}

ScreenDecorator SystemConfigScreen::decorator() const
{
    return ScreenDecorator("SYSTEM CONFIGURATION").withWidth(DEFAULT_PANEL_WIDTH).withPadding(DEFAULT_PADDING);
}
