#include "controllers/UserController.h"

#include <nlohmann/json.hpp>

#include <stdexcept>

namespace
{

// D2: Enriched to include email, full_name, department, designation, manager_id
nlohmann::json userToJson(const User& user)
{
    return {{"id",                    user.id},
            {"username",              user.username},
            {"full_name",             user.fullName},
            {"email",                 user.email},
            {"role",                  user.role},
            {"department",            user.department},
            {"designation",           user.designation},
            {"manager_id",            user.managerId},
            {"status",                user.status},
            {"force_password_change", user.forcePasswordChange},
            {"created_at",            user.createdAt},
            {"updated_at",            user.updatedAt}};
}

int parseUserId(const httplib::Request& req)
{
    if (req.matches.size() < 2)
    {
        throw std::runtime_error("Missing user id path parameter");
    }

    return std::stoi(req.matches[1].str());
}

}  // namespace

UserController::UserController(IUserService& userService) : userService_(userService)
{
}

void UserController::registerRoutes(httplib::Server& server) const
{
    server.Get("/users",
               [&](const httplib::Request&, httplib::Response& res)
               {
                   try
                   {
                       const auto users = userService_.getAllUsers();

                       nlohmann::json data = nlohmann::json::array();
                       for (const auto& user : users)
                       {
                           data.push_back(userToJson(user));
                       }

                       nlohmann::json response;
                       response["success"] = true;
                       response["data"] = data;
                       res.set_content(response.dump(), "application/json");
                   }
                   catch (const std::exception& e)
                   {
                       nlohmann::json response;
                       response["success"] = false;
                       response["message"] = std::string("Internal server error: ") + e.what();
                       res.status = 500;
                       res.set_content(response.dump(), "application/json");
                   }
               });

    server.Get(R"(/users/(\d+))",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   try
                   {
                       const int userId = parseUserId(req);
                       const auto user = userService_.getUserById(userId);

                       if (!user.has_value())
                       {
                           nlohmann::json response;
                           response["success"] = false;
                           response["message"] = "User not found.";
                           res.status = 404;
                           res.set_content(response.dump(), "application/json");
                           return;
                       }

                       nlohmann::json response;
                       response["success"] = true;
                       response["data"] = userToJson(*user);
                       res.set_content(response.dump(), "application/json");
                   }
                   catch (const std::exception& e)
                   {
                       nlohmann::json response;
                       response["success"] = false;
                       response["message"] = std::string("Invalid request: ") + e.what();
                       res.status = 400;
                       res.set_content(response.dump(), "application/json");
                   }
               });

    // D3: Accept optional manager_id when creating a user
    server.Post("/users",
                [&](const httplib::Request& req, httplib::Response& res)
                {
                    try
                    {
                        const auto payload   = nlohmann::json::parse(req.body);
                        const auto username  = payload.at("username").get<std::string>();
                        const auto password  = payload.at("password").get<std::string>();
                        const auto role      = payload.at("role").get<std::string>();
                        const auto email     = payload.at("email").get<std::string>();
                        const auto fullName  = payload.at("full_name").get<std::string>();
                        const auto dept      = payload.value("department", std::string(""));
                        const auto desig     = payload.value("designation", std::string(""));
                        const auto forcePasswordChange =
                            payload.value("force_password_change", true);
                        const int managerId  = payload.value("manager_id", 0);

                        const bool created = userService_.createUser(
                            username, password, role, email, fullName, dept, desig,
                            forcePasswordChange);

                        // If manager_id was provided and creation succeeded, assign it now
                        if (created && managerId > 0)
                        {
                            userService_.assignManager(
                                userService_.getUserByUsername(username).id, managerId);
                        }

                        nlohmann::json response;
                        response["success"] = created;
                        response["message"] = "User created.";
                        res.status = 201;
                        res.set_content(response.dump(), "application/json");
                    }
                    catch (const std::runtime_error& e)
                    {
                        nlohmann::json response;
                        response["success"] = false;
                        response["message"] = e.what();
                        res.status = 400;
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

    server.Put(R"(/users/(\d+)/deactivate)",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   try
                   {
                       const int userId = parseUserId(req);
                       const bool updated = userService_.deactivateUser(userId);

                       nlohmann::json response;
                       response["success"] = updated;
                       response["message"] = updated ? "User deactivated." : "Failed to deactivate user.";
                       res.status = updated ? 200 : 400;
                       res.set_content(response.dump(), "application/json");
                   }
                   catch (const std::exception& e)
                   {
                       nlohmann::json response;
                       response["success"] = false;
                       response["message"] = std::string("Invalid request: ") + e.what();
                       res.status = 400;
                       res.set_content(response.dump(), "application/json");
                   }
               });

    server.Put(R"(/users/(\d+)/reactivate)",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   try
                   {
                       const int userId = parseUserId(req);
                       const bool updated = userService_.reactivateUser(userId);

                       nlohmann::json response;
                       response["success"] = updated;
                       response["message"] = updated ? "User reactivated." : "Failed to reactivate user.";
                       res.status = updated ? 200 : 400;
                       res.set_content(response.dump(), "application/json");
                   }
                   catch (const std::exception& e)
                   {
                       nlohmann::json response;
                       response["success"] = false;
                       response["message"] = std::string("Invalid request: ") + e.what();
                       res.status = 400;
                       res.set_content(response.dump(), "application/json");
                   }
               });

    server.Put(R"(/users/(\d+)/reset-password)",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   try
                   {
                       const int userId = parseUserId(req);
                       const auto payload = nlohmann::json::parse(req.body);
                       const auto newPassword = payload.at("new_password").get<std::string>();
                       const bool forcePasswordChange =
                           payload.value("force_password_change", true);

                       const bool updated = userService_.resetPassword(
                           userId, newPassword, forcePasswordChange);

                       nlohmann::json response;
                       response["success"] = updated;
                       response["message"] =
                           updated ? "Password reset completed." : "Password reset failed.";
                       res.status = updated ? 200 : 400;
                       res.set_content(response.dump(), "application/json");
                   }
                   catch (const std::exception& e)
                   {
                       nlohmann::json response;
                       response["success"] = false;
                       response["message"] = std::string("Invalid request: ") + e.what();
                       res.status = 400;
                       res.set_content(response.dump(), "application/json");
                   }
               });

    // D1: Assign (or unassign) a manager to a user
    server.Put(R"(/users/(\d+)/assign-manager)",
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   try
                   {
                       const int userId    = parseUserId(req);
                       const auto payload  = nlohmann::json::parse(req.body);
                       const int managerId = payload.value("manager_id", 0);

                       const bool updated = userService_.assignManager(userId, managerId);

                       nlohmann::json response;
                       response["success"] = updated;
                       response["message"] =
                           updated ? "Manager assigned." : "Failed to assign manager.";
                       res.status = updated ? 200 : 400;
                       res.set_content(response.dump(), "application/json");
                   }
                   catch (const std::exception& e)
                   {
                       nlohmann::json response;
                       response["success"] = false;
                       response["message"] = std::string("Invalid request: ") + e.what();
                       res.status = 400;
                       res.set_content(response.dump(), "application/json");
                   }
               });
}
