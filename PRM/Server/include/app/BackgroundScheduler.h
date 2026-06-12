#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "services/SchedulerService.h"
#include "repositories/ISystemConfigRepository.h"

/// Runs SchedulerService on a background thread at a fixed interval (hours).
class BackgroundScheduler
{
   public:
    BackgroundScheduler(std::shared_ptr<SchedulerService> schedulerService, std::shared_ptr<ISystemConfigRepository> configRepo);
    ~BackgroundScheduler();

    void start();
    void stop();

   private:
    void loop();

    std::shared_ptr<SchedulerService> schedulerService_;
    std::shared_ptr<ISystemConfigRepository> configRepo_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};
