#include "controllers/SchedulerController.h"

#include <nlohmann/json.hpp>

SchedulerController::SchedulerController(SchedulerService& schedulerService)
    : schedulerService_(schedulerService)
{
}

void SchedulerController::registerRoutes(httplib::Server& server) const
{
    server.Post("/scheduler/recompute",
                [&](const httplib::Request& req, httplib::Response& res)
                {
                    const auto body = req.body.empty() ? nlohmann::json::object()
                                                       : nlohmann::json::parse(req.body);
                    const std::string todayDate = body.value("today", "");
                    schedulerService_.runRecomputationJob(todayDate);

                    res.set_content(nlohmann::json({{"success", true},
                                                    {"message", "Recomputation completed."}})
                                        .dump(),
                                    "application/json");
                });
}
