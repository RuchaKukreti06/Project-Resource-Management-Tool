#include "utils/GlobalExceptionHandler.h"
#include "exceptions/Exceptions.h"
#include <nlohmann/json.hpp>

namespace utils
{

void GlobalExceptionHandler::registerGlobalExceptionHandler(httplib::Server& server)
{
    server.set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
        int status_code = 500;
        std::string error_message = "Internal Server Error";

        try {
            std::rethrow_exception(ep);
        } catch (const exceptions::ValidationException& e) {
            status_code = 400;
            error_message = e.what();
        } catch (const exceptions::AuthenticationException& e) {
            status_code = 401;
            error_message = e.what();
        } catch (const exceptions::AuthorizationException& e) {
            status_code = 403;
            error_message = e.what();
        } catch (const exceptions::NotFoundException& e) {
            status_code = 404;
            error_message = e.what();
        } catch (const exceptions::ConflictException& e) {
            status_code = 409;
            error_message = e.what();
        } catch (const exceptions::DatabaseException& e) {
            status_code = 500;
            error_message = e.what();
        } catch (const exceptions::AppException& e) {
            status_code = 500;
            error_message = e.what();
        } catch (const nlohmann::json::exception& e) {
            status_code = 400;
            error_message = std::string("Invalid JSON payload: ") + e.what();
        } catch (const std::exception& e) {
            status_code = 500;
            error_message = e.what();
        } catch (...) {
            status_code = 500;
            error_message = "Unknown exception occurred";
        }

        res.status = status_code;
        res.set_content(
            nlohmann::json({
                {"success", false},
                {"error", error_message},
                {"code", status_code}
            }).dump(),
            "application/json"
        );
    });
}

} // namespace utils
