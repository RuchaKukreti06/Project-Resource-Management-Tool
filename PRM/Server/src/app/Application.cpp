#include "Application.h"

#include "AuthService.h"
#include "ConfigLoader.h"
#include "Database.h"
#include "httplib.h"

Application::Application()
{
}

bool Application::run()
{
    try
    {
        auto& config = utils::ConfigLoader::instance();
        config.load("PRM/Server/config/config.json");

        auto& database = database::Database::instance();
        database.connect(config.dbHost(), config.dbPort(), config.dbUser(), config.dbPassword(),
                         config.dbName());

        AuthService authService;
        httplib::Server server;

        server.Post("/auth/login",
                    [&](const httplib::Request& req, httplib::Response& res)
                    {
                        try
                        {
                            auto payload = nlohmann::json::parse(req.body);
                            auto username = payload.at("username").get<std::string>();
                            auto password = payload.at("password").get<std::string>();
                            auto response = authService.login(username, password);
                            res.set_content(response.dump(), "application/json");
                        }
                        catch (const std::exception& e)
                        {
                            nlohmann::json response;
                            response["success"] = false;
                            response["message"] = std::string("Invalid request body: ") + e.what();
                            res.status = 400;
                            res.set_content(response.dump(), "application/json");
                        }
                    });

        server.Post("/auth/register",
                    [&](const httplib::Request& req, httplib::Response& res)
                    {
                        try
                        {
                            auto payload = nlohmann::json::parse(req.body);
                            auto username = payload.at("username").get<std::string>();
                            auto password = payload.at("password").get<std::string>();
                            auto response = authService.registerUser(username, password);
                            res.set_content(response.dump(), "application/json");
                        }
                        catch (const std::exception& e)
                        {
                            nlohmann::json response;
                            response["success"] = false;
                            response["message"] = std::string("Invalid request body: ") + e.what();
                            res.status = 400;
                            res.set_content(response.dump(), "application/json");
                        }
                    });

        server.Get("/health", [&](const httplib::Request&, httplib::Response& res)
                   { res.set_content("OK", "text/plain"); });

        std::cout << "Application started on " << config.serverHost() << ":" << config.serverPort()
                  << "\n";
        if (!server.listen(config.serverHost().c_str(), config.serverPort()))
        {
            std::cerr << "Failed to start HTTP server.\n";
            return 1;
        }

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Startup error: " << ex.what() << "\n";
        return 1;
    }
}
