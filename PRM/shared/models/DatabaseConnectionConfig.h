#include <string>

struct DatabaseConnectionConfig
{
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string databaseName;
};