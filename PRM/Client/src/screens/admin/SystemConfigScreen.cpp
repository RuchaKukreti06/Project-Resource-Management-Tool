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
        }
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to load system config: ") + ex.what());
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
    // clearScreen();
    decorator().render();

    std::cout << "\nCurrent Settings:\n";
    std::cout << "  LLM Provider        :  " << llmProvider_ << "\n";
    std::cout << "  LLM API Key         :  "
              << (llmApiKey_.empty() ? "(not set)" : std::string(llmApiKey_.size(), '*')) << "\n";
    std::cout << "  Scheduler Interval  :  " << schedulerInterval_ << " hours\n";
    std::cout << "  Max Weekly Hours    :  " << maxWeeklyHours_ << "\n";
    ScreenUtils::printDivider();

    std::cout << "1. Update LLM API Key\n";
    std::cout << "2. Change LLM Provider (Gemini / Groq / Gemma Remote)\n";
    std::cout << "3. Update Scheduler Interval\n";
    std::cout << "4. Update Max Weekly Hours\n";
    std::cout << "5. Back\n";
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
            showSuccess("LLM Provider updated to " + providerName + ". \u2713");
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
                showSuccess("Scheduler Interval updated to " + std::to_string(hrs) + " hours. \u2713");
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
                showSuccess("Max Weekly Hours updated to " + std::to_string(h) + ". \u2713");
        }
        catch (...)
        {
            showError("Invalid number.");
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    else if (choice == "5" || choice == "B" || choice == "b")
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1\u20135.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator SystemConfigScreen::decorator() const
{
    return ScreenDecorator("SYSTEM CONFIGURATION").withWidth(40).withPadding(2);
}
