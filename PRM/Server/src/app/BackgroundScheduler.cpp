#include "BackgroundScheduler.h"

#include <chrono>
#include <ctime>
#include <spdlog/spdlog.h>

#include "utils/DateUtils.h"

BackgroundScheduler::BackgroundScheduler(std::function<void()> job, std::function<int()> getIntervalHrs)
    : job_(std::move(job)), getIntervalHrs_(std::move(getIntervalHrs))
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
        if (job_) job_();
    }
    catch (const std::exception& ex)
    {
        spdlog::error("BackgroundScheduler initial run failed: {}", ex.what());
    }

    while (running_)
    {
        int intervalHours = 24;
        if (getIntervalHrs_)
        {
            intervalHours = getIntervalHrs_();
        }
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
        if (job_) job_();
        }
        catch (const std::exception& ex)
        {
            spdlog::error("BackgroundScheduler run failed: {}", ex.what());
        }
    }

    spdlog::info("BackgroundScheduler stopped.");
}
