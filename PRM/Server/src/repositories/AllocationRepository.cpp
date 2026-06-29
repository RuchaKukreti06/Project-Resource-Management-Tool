#include "repositories/AllocationRepository.h"

#include <mysqlx/xdevapi.h>
#include <spdlog/spdlog.h>

namespace
{

Allocation mapAllocationRow(const mysqlx::Row& row)
{
    Allocation allocation;
    allocation.id = row[0].get<int>();
    allocation.employeeId = row[1].get<int>();
    allocation.projectId = row[2].get<int>();
    allocation.utilizationPercentage = row[3].get<int>();
    allocation.fromDate = row[4].get<std::string>();
    allocation.toDate = row[5].isNull() ? "" : row[5].get<std::string>();
    return allocation;
}

}  // namespace

AllocationRepository::AllocationRepository(database::Database& database) : database_(database)
{
}

bool AllocationRepository::createAllocation(const Allocation& allocation, int createdByUserId)
{
    try
    {
        // D6: DB columns are resource_id and utilisation_percent (not employee_id / utilization_percentage)
        database_.getSession()
            .sql("INSERT INTO allocations (resource_id, project_id, utilisation_percent, from_date, to_date) "
                 "VALUES (?, ?, ?, ?, ?)")
            .bind(allocation.employeeId)
            .bind(allocation.projectId)
            .bind(allocation.utilizationPercentage)
            .bind(allocation.fromDate)
            .bind(allocation.toDate)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("createAllocation failed: {}", e.what());
        return false;
    }
}

bool AllocationRepository::endAllocation(int allocationId, const std::string& endDate)
{
    try
    {
        database_.getSession()
            .sql("UPDATE allocations SET to_date = ? WHERE id = ?")
            .bind(endDate)
            .bind(allocationId)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("endAllocation failed: {}", e.what());
        return false;
    }
}

bool AllocationRepository::endActiveAllocationsByEmployee(int employeeId,
                                                          const std::string& endDate)
{
    try
    {
        database_.getSession()
            .sql("UPDATE allocations SET to_date = ? "
                 "WHERE resource_id = ? AND (to_date IS NULL OR to_date >= ?)")
            .bind(endDate)
            .bind(employeeId)
            .bind(endDate)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("endActiveAllocationsByEmployee failed: {}", e.what());
        return false;
    }
}

std::vector<Allocation> AllocationRepository::getActiveAllocationsByProject(int projectId)
{
    std::vector<Allocation> allocations;
    try
    {
        // D6: correct column names
        auto result = database_.getSession()
                          .sql("SELECT id, resource_id, project_id, utilisation_percent, DATE_FORMAT(from_date, '%Y-%m-%d'), COALESCE(DATE_FORMAT(to_date, '%Y-%m-%d'), '') "
                               "FROM allocations "
                               "WHERE project_id = ? AND (to_date IS NULL OR to_date >= CURDATE())")
                          .bind(projectId)
                          .execute();

        while (auto row = result.fetchOne())
        {
            allocations.push_back(mapAllocationRow(row));
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getActiveAllocationsByProject failed: {}", e.what());
    }

    return allocations;
}

std::vector<Allocation> AllocationRepository::getOverlappingAllocations(int employeeId,
                                                                        const std::string& fromDate,
                                                                        const std::string& toDate)
{
    std::vector<Allocation> allocations;
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT id, resource_id, project_id, utilisation_percent, DATE_FORMAT(from_date, '%Y-%m-%d'), COALESCE(DATE_FORMAT(to_date, '%Y-%m-%d'), '') "
                               "FROM allocations "
                               "WHERE resource_id = ? AND from_date <= ? "
                               "AND (to_date IS NULL OR to_date >= ?)")
                          .bind(employeeId)
                          .bind(toDate)
                          .bind(fromDate)
                          .execute();

        while (auto row = result.fetchOne())
        {
            allocations.push_back(mapAllocationRow(row));
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getOverlappingAllocations failed: {}", e.what());
    }

    return allocations;
}

int AllocationRepository::getOverlappingUtilization(int employeeId, const std::string& fromDate,
                                                    const std::string& toDate)
{
    try
    {
        auto result = database_.getSession()
                          .sql(
                              "SELECT CAST(COALESCE(SUM(utilisation_percent), 0) AS UNSIGNED) "
                              "FROM allocations "
                              "WHERE resource_id = ? AND from_date <= ? "
                              "AND (to_date IS NULL OR to_date >= ?)")
                          .bind(employeeId)
                          .bind(toDate)
                          .bind(fromDate)
                          .execute();

        if (auto row = result.fetchOne())
        {
            return static_cast<int>(row[0].get<uint64_t>());
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getOverlappingUtilization failed: {}", e.what());
    }

    return 0;
}

int AllocationRepository::getCurrentUtilization(int employeeId, const std::string& date)
{
    try
    {
        auto result = database_.getSession()
                          .sql(
                              "SELECT CAST(COALESCE(SUM(utilisation_percent), 0) AS UNSIGNED) "
                              "FROM allocations "
                              "WHERE resource_id = ? AND from_date <= ? "
                              "AND (to_date IS NULL OR to_date >= ?)")
                          .bind(employeeId)
                          .bind(date)
                          .bind(date)
                          .execute();

        if (auto row = result.fetchOne())
        {
            return static_cast<int>(row[0].get<uint64_t>());
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getCurrentUtilization failed: {}", e.what());
    }

    return 0;
}

std::vector<Allocation> AllocationRepository::getAllocationsByEmployee(int employeeId)
{
    std::vector<Allocation> allocations;
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT id, resource_id, project_id, utilisation_percent, DATE_FORMAT(from_date, '%Y-%m-%d'), COALESCE(DATE_FORMAT(to_date, '%Y-%m-%d'), '') "
                               "FROM allocations "
                               "WHERE resource_id = ? "
                               "ORDER BY from_date DESC")
                          .bind(employeeId)
                          .execute();

        while (auto row = result.fetchOne())
        {
            allocations.push_back(mapAllocationRow(row));
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getAllocationsByEmployee failed: {}", e.what());
    }

    return allocations;
}

std::optional<Allocation> AllocationRepository::getAllocationById(int allocationId)
{
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT id, resource_id, project_id, utilisation_percent, DATE_FORMAT(from_date, '%Y-%m-%d'), COALESCE(DATE_FORMAT(to_date, '%Y-%m-%d'), '') "
                               "FROM allocations "
                               "WHERE id = ?")
                          .bind(allocationId)
                          .execute();

        if (auto row = result.fetchOne())
        {
            return mapAllocationRow(row);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getAllocationById failed: {}", e.what());
    }

    return std::nullopt;
}
