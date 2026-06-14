#include "services/EmailService.h"

#include <curl/curl.h>

#include <cstring>
#include <sstream>
#include <utility>

EmailService::EmailService(std::shared_ptr<ISystemConfigRepository> configRepository)
    : configRepository_(std::move(configRepository))
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

EmailService::~EmailService()
{
    curl_global_cleanup();
}

size_t EmailService::payloadSource(char* ptr, size_t size, size_t nmemb, void* userData)
{
    const size_t maxBytes = size * nmemb;
    auto* payload = static_cast<std::pair<const char*, size_t>*>(userData);
    if (payload->second == 0)
    {
        return 0;
    }

    size_t toCopy = payload->second;
    if (toCopy > maxBytes)
    {
        toCopy = maxBytes;
    }

    std::memcpy(ptr, payload->first, toCopy);
    payload->first += toCopy;
    payload->second -= toCopy;
    return toCopy;
}

bool EmailService::sendEmail(const EmailMessage& message, std::string& errorMessage)
{
    if (!configRepository_)
    {
        errorMessage = "Email service is not configured.";
        return false;
    }

    const auto cfg = configRepository_->getConfig();

    if (!cfg.smtpEnabled)
    {
        errorMessage = "SMTP is disabled in system configuration.";
        return false;
    }
    if (cfg.smtpHost.empty() || cfg.smtpUsername.empty() || cfg.smtpPassword.empty() ||
        cfg.smtpFromEmail.empty())
    {
        errorMessage = "SMTP host, credentials, and from email must be configured.";
        return false;
    }
    if (message.to.empty())
    {
        errorMessage = "Recipient email is required.";
        return false;
    }

    std::ostringstream payload;
    const std::string fromDisplay = cfg.smtpFromName.empty() ? cfg.smtpFromEmail
                                                              : (cfg.smtpFromName + " <" + cfg.smtpFromEmail + ">");
    payload << "To: <" << message.to << ">\r\n";
    payload << "From: " << fromDisplay << "\r\n";
    payload << "Subject: " << message.subject << "\r\n";
    payload << "MIME-Version: 1.0\r\n";
    payload << "Content-Type: text/plain; charset=UTF-8\r\n";
    payload << "\r\n";
    payload << message.body << "\r\n";

    const std::string data = payload.str();
    std::pair<const char*, size_t> uploadData = {data.c_str(), data.size()};

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        errorMessage = "Failed to initialize SMTP client.";
        return false;
    }

    struct curl_slist* recipients = nullptr;
    const std::string smtpUrl = "smtp://" + cfg.smtpHost + ":" + std::to_string(cfg.smtpPort);
    const std::string mailFrom = "<" + cfg.smtpFromEmail + ">";
    const std::string recipient = "<" + message.to + ">";

    curl_easy_setopt(curl, CURLOPT_URL, smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, cfg.smtpUsername.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, cfg.smtpPassword.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mailFrom.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, &EmailService::payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA, &uploadData);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    if (cfg.smtpUseTls)
    {
        curl_easy_setopt(curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    }

    recipients = curl_slist_append(recipients, recipient.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    const CURLcode result = curl_easy_perform(curl);

    if (recipients)
    {
        curl_slist_free_all(recipients);
    }
    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
    {
        errorMessage = curl_easy_strerror(result);
        return false;
    }

    return true;
}
