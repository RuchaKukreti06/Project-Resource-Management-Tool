#include "Database.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace database
{

Database& Database::instance()
{
    static Database inst;
    return inst;
}

void Database::connect(const std::string& host, int port, const std::string& user,
                       const std::string& password, const std::string& dbName)
{
    if (connected_)
    {
        return;
    }

    dbName_ = dbName;

    try
    {
        session_ = std::make_unique<mysqlx::Session>(
            mysqlx::SessionOption::HOST, host, mysqlx::SessionOption::PORT, port,
            mysqlx::SessionOption::USER, user, mysqlx::SessionOption::PWD, password);
        connected_ = true;
        spdlog::info("Connected to MySQL at {}:{}/{}", host, port, dbName);
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("MySQL connection failed: {}", e.what());
        throw std::runtime_error("Database connection failed: " + std::string(e.what()));
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

}