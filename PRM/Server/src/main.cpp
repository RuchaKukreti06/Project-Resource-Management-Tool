#include <iostream>
#include "database/Database.h"
 
int main() {
 
    if (!Database::getInstance().connect()) {
        return 1;
    }

    auto& config = utils::ConfigLoader::instance();

    auto& database = database::Database::instance();
    database.connect(config.dbHost(), config.dbPort(), config.dbUser(), config.dbPassword(), config.dbName());

    std::cout << "Application started.\n";
 
    return 0;
}