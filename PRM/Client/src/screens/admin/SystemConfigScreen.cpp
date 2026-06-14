#include "admin/SystemConfigScreen.h"

SystemConfigScreen::SystemConfigScreen()
{
}

void SystemConfigScreen::show(ApiClient& apiClient)
{
    keepRunning_ = true;
    loadConfig(apiClient);
    while (keepRunning_)
    {
        displayMenu();
        handleInput(apiClient);
    }
}

void SystemConfigScreen::loadConfig(ApiClient& apiClient)
{
    try
    {
        auto response = apiClient.get("/system/config");
        if (response["success"].get<bool>() && response.contains("data"))
        {
            const auto& data = response["data"];
            llmProvider_       = data.value("llm_provider", "Google Gemini");
            llmApiKey_         = data.value("llm_api_key", "");
            schedulerInterval_ = data.value("scheduler_interval", 4);
            maxWeeklyHours_    = data.value("max_weekly_hours", 40);
            smtpEnabled_       = data.value("smtp_enabled", false);
            smtpHost_          = data.value("smtp_host", "");
            smtpPort_          = data.value("smtp_port", 587);
            smtpUsername_      = data.value("smtp_username", "");
            smtpPasswordMasked_ = data.value("smtp_password", "");
            smtpFromEmail_     = data.value("smtp_from_email", "");
            smtpFromName_      = data.value("smtp_from_name", "");
            smtpUseTls_        = data.value("smtp_use_tls", true);
        }
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to load system config: ") + ex.what());
    }
}

bool SystemConfigScreen::sendTestEmail(ApiClient& apiClient, const std::string& toEmail)
{
    try
    {
        auto response = apiClient.post(
            "/notifications/test-email",
            {{"to_email", toEmail},
             {"subject", "PRM SMTP Test"},
             {"body", "SMTP configuration test succeeded."}});

        if (!response["success"].get<bool>())
        {
            showError(response["message"].get<std::string>());
            return false;
        }
        return true;
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to send test email: ") + ex.what());
        return false;
    }
}

bool SystemConfigScreen::saveConfig(ApiClient& apiClient, const nlohmann::json& patch)
{
    try
    {
        auto response = apiClient.put("/system/config", patch);
        if (!response["success"].get<bool>())
        {
            showError(response["message"].get<std::string>());
            return false;
        }
        loadConfig(apiClient);  // Refresh local values from server
        return true;
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to save config: ") + ex.what());
        return false;
    }
}

void SystemConfigScreen::displayMenu()
{
    decorator().render();

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

    std::cout << "1. Update LLM API Key\n";
    std::cout << "2. Change LLM Provider (Gemini / Groq / Gemma Remote)\n";
    std::cout << "3. Update Scheduler Interval\n";
    std::cout << "4. Update Max Weekly Hours\n";
    std::cout << "5. Enable/Disable SMTP\n";
    std::cout << "6. Update SMTP Server (Host + Port)\n";
    std::cout << "7. Update SMTP Credentials\n";
    std::cout << "8. Update SMTP Sender\n";
    std::cout << "9. Toggle SMTP TLS\n";
    std::cout << "10. Send Test Email\n";
    std::cout << "11. Back\n";
}

void SystemConfigScreen::handleInput(ApiClient& apiClient)
{
    std::string choice = ScreenUtils::readLine("Enter option");

    if (choice == "1")
    {
        std::string key = ScreenUtils::readLine("Enter new LLM API Key");
        if (key.empty())
        {
            showError("API Key cannot be empty.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }
        if (saveConfig(apiClient, {{"llm_api_key", key}}))
            showSuccess("LLM API Key updated successfully.");
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "2")
    {
        std::cout << "Select Provider:\n";
        std::cout << "1. Google Gemini\n";
        std::cout << "2. Groq\n";
        std::cout << "3. Gemma (Remote) - http://164.52.211.238\n";
        std::string prov = ScreenUtils::readLine("Choice");
        std::string providerName;
        if      (prov == "1") providerName = "Google Gemini";
        else if (prov == "2") providerName = "Groq";
        else if (prov == "3") providerName = "Gemma (Remote)";
        else
        {
            showError("Invalid choice.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }
        if (saveConfig(apiClient, {{"llm_provider", providerName}}))
            showSuccess("LLM Provider updated to " + providerName + ".");
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "3")
    {
        std::string val = ScreenUtils::readLine("Enter Scheduler Interval in hours (1-168)");
        try
        {
            int hrs = std::stoi(val);
            if (hrs < 1 || hrs > 168)
            {
                showError("Interval must be between 1 and 168.");
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }
            if (saveConfig(apiClient, {{"scheduler_interval", hrs}}))
                showSuccess("Scheduler Interval updated to " + std::to_string(hrs) + " hours.");
        }
        catch (...)
        {
            showError("Invalid number.");
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "4")
    {
        std::string hrs = ScreenUtils::readLine("Enter Max Weekly Hours (1-168)");
        try
        {
            int h = std::stoi(hrs);
            if (h < 1 || h > 168)
            {
                showError("Hours must be between 1 and 168.");
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }
            if (saveConfig(apiClient, {{"max_weekly_hours", h}}))
                showSuccess("Max Weekly Hours updated to " + std::to_string(h) + ".");
        }
        catch (...)
        {
            showError("Invalid number.");
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "5")
    {
        if (saveConfig(apiClient, {{"smtp_enabled", !smtpEnabled_}}))
            showSuccess(std::string("SMTP ") + (!smtpEnabled_ ? "enabled." : "disabled."));
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "6")
    {
        std::string host = ScreenUtils::readLine("Enter SMTP host");
        std::string port = ScreenUtils::readLine("Enter SMTP port");
        try
        {
            int parsedPort = std::stoi(port);
            if (parsedPort < 1 || parsedPort > 65535)
            {
                showError("SMTP port must be between 1 and 65535.");
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }

            if (saveConfig(apiClient, {{"smtp_host", host}, {"smtp_port", parsedPort}}))
                showSuccess("SMTP server updated.");
        }
        catch (...)
        {
            showError("Invalid port number.");
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "7")
    {
        std::string username = ScreenUtils::readLine("Enter SMTP username");
        std::string password = ScreenUtils::readLine("Enter SMTP password");
        if (password.empty())
        {
            showError("SMTP password cannot be empty.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }
        if (saveConfig(apiClient, {{"smtp_username", username}, {"smtp_password", password}}))
            showSuccess("SMTP credentials updated.");
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "8")
    {
        std::string fromEmail = ScreenUtils::readLine("Enter SMTP from email");
        std::string fromName = ScreenUtils::readLine("Enter SMTP from name (optional)");
        if (fromEmail.empty())
        {
            showError("SMTP from email cannot be empty.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }
        if (saveConfig(apiClient, {{"smtp_from_email", fromEmail}, {"smtp_from_name", fromName}}))
            showSuccess("SMTP sender updated.");
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "9")
    {
        if (saveConfig(apiClient, {{"smtp_use_tls", !smtpUseTls_}}))
            showSuccess(std::string("SMTP TLS ") + (!smtpUseTls_ ? "enabled." : "disabled."));
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "10")
    {
        std::string toEmail = ScreenUtils::readLine("Enter recipient email");
        if (toEmail.empty())
        {
            showError("Recipient email cannot be empty.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }
        if (sendTestEmail(apiClient, toEmail))
            showSuccess("Test email sent.");
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "11" || choice == "B" || choice == "b")
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1-11.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator SystemConfigScreen::decorator() const
{
    return ScreenDecorator("SYSTEM CONFIGURATION").withWidth(40).withPadding(2);
}
