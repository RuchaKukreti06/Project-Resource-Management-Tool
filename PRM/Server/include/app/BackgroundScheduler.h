#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <functional>

/// Runs a job on a background thread at a fixed interval (hours).
class BackgroundScheduler
{
   public:
    BackgroundScheduler(std::function<void()> job, std::function<int()> getIntervalHrs);
    ~BackgroundScheduler();

    void start();
    void stop();

   private:
    void loop();

    std::function<void()> job_;
    std::function<int()> getIntervalHrs_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};
