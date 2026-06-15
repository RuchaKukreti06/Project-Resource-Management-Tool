#pragma once

#include <gmock/gmock.h>
#include "repositories/ISystemConfigRepository.h"

class MockSystemConfigRepository : public ISystemConfigRepository
{
public:
    MOCK_METHOD(SystemConfig, getConfig, (), (override));
    MOCK_METHOD(bool, updateConfig, (const SystemConfig& config), (override));
};
