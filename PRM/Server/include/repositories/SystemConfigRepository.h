#pragma once

#include "repositories/ISystemConfigRepository.h"
#include "database/Database.h"

class SystemConfigRepository : public ISystemConfigRepository
{
   public:
    explicit SystemConfigRepository(database::Database& db);

    SystemConfig getConfig() override;
    bool         updateConfig(const SystemConfig& config) override;

   private:
    database::Database& db_;
};
