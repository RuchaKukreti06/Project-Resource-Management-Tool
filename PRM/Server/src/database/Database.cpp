#include "Database.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace database
{

Database& Database::instance()
{
    static Database instance;
    return instance;
}

void Database::connect(DatabaseConnectionConfig& databaseConnectionConfig)
{
    if (connected_)
    {
        return;
    }

    dbName_ = databaseConnectionConfig.databaseName;

    try
    {
        session_ = std::make_unique<mysqlx::Session>(
            mysqlx::SessionOption::HOST, databaseConnectionConfig.host, mysqlx::SessionOption::PORT,
            databaseConnectionConfig.port, mysqlx::SessionOption::USER,
            databaseConnectionConfig.user, mysqlx::SessionOption::PWD,
            databaseConnectionConfig.password);
        connected_ = true;
        spdlog::info("Connected to MySQL at {}:{}/{}", databaseConnectionConfig.host,
                     databaseConnectionConfig.port, databaseConnectionConfig.databaseName);
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("MySQL connection failed: {}", error.what());
        throw std::runtime_error("Database connection failed: " + std::string(error.what()));
    }
}

mysqlx::Session& Database::getSession()
{
    if (!connected_ || !session_)
    {
        throw std::runtime_error("Database not connected");
    }
    return *session_;
}

mysqlx::Schema Database::getSchema()
{
    return getSession().getSchema(dbName_);
}

bool Database::isConnected() const
{
    return connected_;
}

void Database::disconnect()
{
    if (session_)
    {
        session_->close();
        session_.reset();
    }
    connected_ = false;
    spdlog::info("Disconnected from MySQL");
}

}  // namespace database
