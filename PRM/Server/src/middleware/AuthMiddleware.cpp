#include "middleware/AuthMiddleware.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

AuthMiddleware::AuthMiddleware(ITokenService& tokenService) : tokenService_(tokenService)
{
}

void AuthMiddleware::registerMiddleware(httplib::Server& server)
{
    server.set_pre_routing_handler(
        [this](const httplib::Request& req, httplib::Response& res) -> httplib::Server::HandlerResponse
        {
            // Allow public routes
            std::vector<std::string> publicRoutes = {"/auth/login", "/auth/register", "/health"};

            for (const auto& route : publicRoutes)
            {
                if (req.path == route)
                {
                    return httplib::Server::HandlerResponse::Unhandled;
                }
            }

            if (req.path.find("/docs") == 0)
            {
                return httplib::Server::HandlerResponse::Unhandled;
            }

            // All other routes require Authorization
            std::string authHeader = req.get_header_value("Authorization");
            if (authHeader.empty() || authHeader.find("Bearer ") != 0)
            {
                res.status = 401;
                res.set_content(
                    nlohmann::json({{"success", false}, {"error", "Unauthorized"}}).dump(),
                    "application/json");
                return httplib::Server::HandlerResponse::Handled;
            }

            std::string token = authHeader.substr(7);
            if (!tokenService_.validateToken(token))
            {
                res.status = 401;
                res.set_content(
                    nlohmann::json({{"success", false}, {"error", "Invalid or expired token"}}).dump(),
                    "application/json");
                return httplib::Server::HandlerResponse::Handled;
            }

            if (tokenService_.getClaimForcePasswordChange(token) && req.path != "/auth/change-password")
            {
                res.status = 403;
                res.set_content(
                    nlohmann::json({{"success", false}, {"error", "Password change required."}}).dump(),
                    "application/json");
                return httplib::Server::HandlerResponse::Handled;
            }

            // Minimal RBAC enforcement
            std::string role = tokenService_.getClaimRole(token);

            auto respondForbidden = [&res]() {
                res.status = 403;
                res.set_content(
                    nlohmann::json({{"success", false}, {"error", "Forbidden: Insufficient role."}}).dump(),
                    "application/json");
                return httplib::Server::HandlerResponse::Handled;
            };

            // ADMIN only
            if (req.path.find("/users") == 0 || req.path.find("/system/config") == 0)
            {
                if (role != "ADMIN") return respondForbidden();
            }
            // ADMIN or MANAGER
            else if (req.path.find("/scheduler") == 0 || 
                     req.path.find("/timesheets/access/restore") == 0)
            {
                if (role != "ADMIN" && role != "MANAGER") return respondForbidden();
            }
            else if (req.path.find("/ai") == 0)
            {
                if (role != "MANAGER") return respondForbidden();
            }
            else if (req.path.find("/employees") == 0)
            {
                if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE")
                {
                    if (role != "ADMIN") return respondForbidden();
                }
            }
            else if (req.path.find("/projects") == 0 || 
                     req.path.find("/milestones") == 0)
            {
                if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE")
                {
                    if (role != "ADMIN") return respondForbidden();
                }
            }
            else if (req.path.find("/allocations") == 0)
            {
                if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE")
                {
                    if (role != "MANAGER") return respondForbidden();
                }
            }

            return httplib::Server::HandlerResponse::Unhandled;
        });
}
