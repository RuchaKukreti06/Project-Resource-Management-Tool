#ifndef PROJECT_RESOURCE_MANAGEMENT_TOOL_SERVER_DATABASE_DATABASE_H
#define PROJECT_RESOURCE_MANAGEMENT_TOOL_SERVER_DATABASE_DATABASE_H

#include <mysqlx/xdevapi.h>

#include <memory>
#include <string>

#include "DatabaseConnectionConfig.h"

namespace database
{

class Database
{
   public:
    static Database& instance();

    void connect(DatabaseConnectionConfig& databaseConnectionConfig);
    void loadDatabaseConnectionConfig(DatabaseConnectionConfig databaseConnectionConfig);
    mysqlx::Session& getSession();
    mysqlx::Schema getSchema();

    bool isConnected() const;
    void disconnect();

   private:
    Database() = default;
    ~Database() = default;
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    std::unique_ptr<mysqlx::Session> session_;
    std::string dbName_;
    bool connected_ = false;
};

}  // namespace database

#endif
