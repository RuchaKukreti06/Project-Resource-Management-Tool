#include "BackgroundScheduler.h"

#include <chrono>
#include <ctime>
#include <spdlog/spdlog.h>

namespace
{
std::string currentDateIso()
{
    std::time_t now = std::time(nullptr);
    std::tm local  = {};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    local = *std::localtime(&now);
#endif
    char buffer[11] = {0};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local);
    return buffer;
}
}  // namespace

BackgroundScheduler::BackgroundScheduler(std::shared_ptr<SchedulerService> schedulerService,
                                         std::shared_ptr<ISystemConfigRepository> configRepo)
    : schedulerService_(std::move(schedulerService)), configRepo_(std::move(configRepo))
{
}

BackgroundScheduler::~BackgroundScheduler()
{
    stop();
}

void BackgroundScheduler::start()
{
    running_ = true;
    thread_  = std::thread(&BackgroundScheduler::loop, this);
    spdlog::info("BackgroundScheduler started");
}

void BackgroundScheduler::stop()
{
    running_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void BackgroundScheduler::loop()
{
    // Run once immediately on start
    try
    {
        schedulerService_->runRecomputationJob(currentDateIso());
    }
    catch (const std::exception& ex)
    {
        spdlog::error("BackgroundScheduler initial run failed: {}", ex.what());
    }

    while (running_)
    {
        int intervalHours = configRepo_->getConfig().schedulerIntervalHrs;
        auto interval = std::chrono::hours(intervalHours);
        auto nextRun  = std::chrono::steady_clock::now() + interval;

        // Sleep in small chunks so stop() is responsive
        while (running_ && std::chrono::steady_clock::now() < nextRun)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        if (!running_)
            break;

        try
        {
            schedulerService_->runRecomputationJob(currentDateIso());
        }
        catch (const std::exception& ex)
        {
            spdlog::error("BackgroundScheduler run failed: {}", ex.what());
        }
    }

    spdlog::info("BackgroundScheduler stopped.");
}
