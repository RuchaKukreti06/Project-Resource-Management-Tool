#include "Application.h"

#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>

#include "BackgroundScheduler.h"
#include "DatabaseConnectionConfig.h"
#include "controllers/AIController.h"
#include "controllers/AllocationController.h"
#include "controllers/AuthController.h"
#include "controllers/EmployeeController.h"
#include "controllers/ProjectController.h"
#include "controllers/SchedulerController.h"
#include "controllers/TimesheetController.h"
#include "controllers/UserController.h"
#include "database/Database.h"
#include "httplib.h"
#include "repositories/AllocationRepository.h"
#include "repositories/EmployeeRepository.h"
#include "repositories/ProjectRepository.h"
#include "repositories/NotificationRepository.h"
#include "repositories/TimesheetRepository.h"
#include "repositories/UserRepository.h"
#include "repositories/SystemConfigRepository.h"
#include "services/AIService.h"
#include "services/AllocationService.h"
#include "services/AuthService.h"
#include "services/PasswordHasher.h"
#include "services/JwtTokenService.h"
#include "services/EmployeeService.h"
#include "services/EmailService.h"
#include "services/NotificationService.h"
#include "services/ProjectService.h"
#include "services/SchedulerService.h"
#include "services/TimesheetService.h"
#include "services/UserService.h"
#include "utils/ConfigLoader.h"

Application::Application()
{
}

bool Application::run()
{
    try
    {
        auto& config = utils::ConfigLoader::instance();
        DatabaseConnectionConfig databaseConnectionConfig;
        const std::string configPath = std::filesystem::exists("Server/config/config.json")
                                           ? "Server/config/config.json"
                                           : "PRM/Server/config/config.json";
        config.load(configPath);

        databaseConnectionConfig.host         = config.dbHost();
        databaseConnectionConfig.port         = config.dbPort();
        databaseConnectionConfig.user         = config.dbUser();
        databaseConnectionConfig.password     = config.dbPassword();
        databaseConnectionConfig.databaseName = config.dbName();

        auto& database = database::Database::instance();
        database.connect(databaseConnectionConfig);

        // ── Repositories ──────────────────────────────────────────────────────
        auto userRepository       = std::make_shared<UserRepository>(database);
        auto employeeRepository   = std::make_shared<EmployeeRepository>(database);
        auto projectRepository    = std::make_shared<ProjectRepository>(database);
        auto allocationRepository = std::make_shared<AllocationRepository>(database);
        auto timesheetRepository  = std::make_shared<TimesheetRepository>(database);
        auto notificationRepository = std::make_shared<NotificationRepository>(database);
        auto systemConfigRepository = std::make_shared<SystemConfigRepository>(database);

        // ── Services ──────────────────────────────────────────────────────────
        auto passwordHasher = std::make_shared<PasswordHasher>();
        AuthConfig  authConfig{config.jwtSecret(), config.jwtExpirationMinutes()};
        auto tokenService = std::make_shared<JwtTokenService>(authConfig);
        AuthService authService(userRepository, passwordHasher, tokenService);
        UserService userService(userRepository, passwordHasher);
        auto employeeService  = std::make_shared<EmployeeService>(employeeRepository,
                                                                   userRepository,
                                                                   allocationRepository);
        auto projectService   = std::make_shared<ProjectService>(projectRepository, userRepository);
        auto allocationService = std::make_shared<AllocationService>(allocationRepository,
                                                                      employeeRepository,
                                                                      projectRepository);
        auto emailService = std::make_shared<EmailService>(systemConfigRepository);
        auto notificationService = std::make_shared<NotificationService>(notificationRepository,
                                                                         userRepository,
                                                                         emailService);
        auto timesheetService = std::make_shared<TimesheetService>(timesheetRepository,
                                                                    employeeRepository,
                                                           allocationRepository,
                                                           notificationService);
        auto schedulerService = std::make_shared<SchedulerService>(employeeService,
                                                                    projectService,
                                                                    allocationService,
                                                           timesheetService,
                                                           notificationService);
           auto aiService = std::make_shared<AIService>(employeeRepository,
                                                      allocationRepository,
                                                      projectRepository,
                                                      timesheetRepository);

        // ── Controllers ───────────────────────────────────────────────────────
        AuthController      authController(authService);
        UserController      userController(userService);
        EmployeeController  employeeController(*employeeService);
        ProjectController   projectController(*projectService);
        AllocationController allocationController(*allocationService);
        TimesheetController  timesheetController(*timesheetService, *notificationService);
        SchedulerController  schedulerController(*schedulerService);
        AIController         aiController(aiService, systemConfigRepository);

        httplib::Server server;
        authController.registerRoutes(server);
        userController.registerRoutes(server);
        employeeController.registerRoutes(server);
        projectController.registerRoutes(server);
        allocationController.registerRoutes(server);
        timesheetController.registerRoutes(server);
        schedulerController.registerRoutes(server);
        aiController.registerRoutes(server);

        // ── API Documentation (Swagger) ───────────────────────────────────────
        const std::string docsPath = std::filesystem::exists("Server/docs")
                                         ? "Server/docs"
                                         : "PRM/Server/docs";
        server.set_mount_point("/docs", docsPath.c_str());

        // ── System Config Endpoints ───────────────────────────────────────────
        server.Get("/system/config",
                   [&, systemConfigRepository](const httplib::Request&, httplib::Response& res)
                   {
                       auto sysCfg = systemConfigRepository->getConfig();
                       nlohmann::json data = {
                           {"llm_provider",        sysCfg.llmProvider},
                           {"llm_api_key",         sysCfg.llmApiKey.empty() ? "" : "****"},
                           {"scheduler_interval",  sysCfg.schedulerIntervalHrs},
                           {"max_weekly_hours",    sysCfg.maxWeeklyHours},
                           {"smtp_enabled",        sysCfg.smtpEnabled},
                           {"smtp_host",           sysCfg.smtpHost},
                           {"smtp_port",           sysCfg.smtpPort},
                           {"smtp_username",       sysCfg.smtpUsername},
                           {"smtp_password",       sysCfg.smtpPassword.empty() ? "" : "****"},
                           {"smtp_from_email",     sysCfg.smtpFromEmail},
                           {"smtp_from_name",      sysCfg.smtpFromName},
                           {"smtp_use_tls",        sysCfg.smtpUseTls}
                       };
                       res.set_content(
                           nlohmann::json({{"success", true}, {"data", data}}).dump(),
                           "application/json");
                   });

        server.Put("/system/config",
                   [&, systemConfigRepository](const httplib::Request& req, httplib::Response& res)
                   {
                       try
                       {
                           const auto body = nlohmann::json::parse(req.body);
                           auto sysCfg = systemConfigRepository->getConfig();

                           if (body.contains("llm_api_key"))
                               sysCfg.llmApiKey = body["llm_api_key"].get<std::string>();
                           if (body.contains("llm_provider"))
                               sysCfg.llmProvider = body["llm_provider"].get<std::string>();
                           if (body.contains("max_weekly_hours"))
                               sysCfg.maxWeeklyHours = body["max_weekly_hours"].get<int>();
                           if (body.contains("scheduler_interval"))
                               sysCfg.schedulerIntervalHrs = body["scheduler_interval"].get<int>();
                           if (body.contains("smtp_enabled"))
                               sysCfg.smtpEnabled = body["smtp_enabled"].get<bool>();
                           if (body.contains("smtp_host"))
                               sysCfg.smtpHost = body["smtp_host"].get<std::string>();
                           if (body.contains("smtp_port"))
                               sysCfg.smtpPort = body["smtp_port"].get<int>();
                           if (body.contains("smtp_username"))
                               sysCfg.smtpUsername = body["smtp_username"].get<std::string>();
                           if (body.contains("smtp_password"))
                               sysCfg.smtpPassword = body["smtp_password"].get<std::string>();
                           if (body.contains("smtp_from_email"))
                               sysCfg.smtpFromEmail = body["smtp_from_email"].get<std::string>();
                           if (body.contains("smtp_from_name"))
                               sysCfg.smtpFromName = body["smtp_from_name"].get<std::string>();
                           if (body.contains("smtp_use_tls"))
                               sysCfg.smtpUseTls = body["smtp_use_tls"].get<bool>();

                           systemConfigRepository->updateConfig(sysCfg);

                           res.set_content(
                               nlohmann::json({{"success", true},
                                               {"message", "Configuration updated."}}).dump(),
                               "application/json");
                       }
                       catch (const std::exception& e)
                       {
                           res.status = 400;
                           res.set_content(
                               nlohmann::json({{"success", false}, {"message", e.what()}}).dump(),
                               "application/json");
                       }
                   });

        server.Post("/notifications/test-email",
                    [emailService](const httplib::Request& req, httplib::Response& res)
                    {
                        try
                        {
                            const auto body = nlohmann::json::parse(req.body);
                            EmailMessage message;
                            message.to = body.at("to_email").get<std::string>();
                            message.subject = body.value("subject", std::string("PRM Test Email"));
                            message.body = body.value("body", std::string("SMTP configuration is working."));

                            std::string errorMessage;
                            const bool sent = emailService->sendEmail(message, errorMessage);

                            if (sent)
                            {
                                res.set_content(
                                    nlohmann::json({{"success", true},
                                                    {"message", "Test email sent successfully."}})
                                        .dump(),
                                    "application/json");
                                return;
                            }

                            res.status = 400;
                            res.set_content(
                                nlohmann::json({{"success", false}, {"message", errorMessage}})
                                    .dump(),
                                "application/json");
                        }
                        catch (const std::exception& e)
                        {
                            res.status = 400;
                            res.set_content(
                                nlohmann::json({{"success", false}, {"message", e.what()}}).dump(),
                                "application/json");
                        }
                    });

        server.Get("/health",
                   [&](const httplib::Request&, httplib::Response& res)
                   {
                       nlohmann::json health = {{"status", "OK"}};
                       res.set_content(health.dump(), "application/json");
                   });

        // ── Background Scheduler ──────────────────────────────────────────────
        BackgroundScheduler bgScheduler(schedulerService, systemConfigRepository);
        bgScheduler.start();

        std::cout << "Application started on " << config.serverHost() << ":" << config.serverPort()
                  << "\n";
        if (!server.listen(config.serverHost().c_str(), config.serverPort()))
        {
            std::cerr << "Failed to start HTTP server.\n";
            return false;
        }

        bgScheduler.stop();
        return true;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Startup error: " << ex.what() << "\n";
        return false;
    }
}
