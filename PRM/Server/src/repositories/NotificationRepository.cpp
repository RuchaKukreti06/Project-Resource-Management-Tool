#include "repositories/NotificationRepository.h"

#include <spdlog/spdlog.h>

#include <string>

NotificationRepository::NotificationRepository(database::Database& database)
    : database_(database)
{
    ensureSchema();
}

void NotificationRepository::ensureSchema()
{
    try
    {
        database_.getSession()
            .sql("CREATE TABLE IF NOT EXISTS timesheet_notification_state ("
                 "id INT AUTO_INCREMENT PRIMARY KEY, "
                 "user_id INT NOT NULL, "
                 "week_start_date DATE NOT NULL, "
                 "notification_stage INT NOT NULL DEFAULT 0, "
                 "is_locked BOOLEAN NOT NULL DEFAULT FALSE, "
                 "is_restored BOOLEAN NOT NULL DEFAULT FALSE, "
                 "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
                 "UNIQUE KEY uq_timesheet_notification_state_user_week (user_id, week_start_date), "
                 "CONSTRAINT fk_timesheet_notification_state_user "
                 "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE)")
            .execute();
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to ensure notification schema: {}", e.what());
    }
}

int NotificationRepository::getNotificationStage(int userId, const std::string& weekStartDate)
{
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT notification_stage FROM timesheet_notification_state "
                               "WHERE user_id = ? AND week_start_date = ?")
                          .bind(userId)
                          .bind(weekStartDate)
                          .execute();

        if (auto row = result.fetchOne())
        {
            return row[0].get<int>();
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to get notification stage: {}", e.what());
    }
    return 0;
}

bool NotificationRepository::setNotificationStage(int userId, const std::string& weekStartDate,
                                                  int stage)
{
    try
    {
        database_.getSession()
            .sql("INSERT INTO timesheet_notification_state "
                 "(user_id, week_start_date, notification_stage, is_locked, is_restored) "
                 "VALUES (?, ?, ?, FALSE, FALSE) "
                 "ON DUPLICATE KEY UPDATE notification_stage = VALUES(notification_stage)")
            .bind(userId)
            .bind(weekStartDate)
            .bind(stage)
            .execute();
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to set notification stage: {}", e.what());
        return false;
    }
}

bool NotificationRepository::lockTimesheetAccess(int userId, const std::string& weekStartDate)
{
    try
    {
        database_.getSession()
            .sql("INSERT INTO timesheet_notification_state "
                 "(user_id, week_start_date, notification_stage, is_locked, is_restored) "
                 "VALUES (?, ?, 3, TRUE, FALSE) "
                 "ON DUPLICATE KEY UPDATE notification_stage = 3, is_locked = TRUE, "
                 "is_restored = FALSE")
            .bind(userId)
            .bind(weekStartDate)
            .execute();
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to lock timesheet access: {}", e.what());
        return false;
    }
}

bool NotificationRepository::restoreTimesheetAccess(int userId, const std::string& weekStartDate)
{
    try
    {
        database_.getSession()
            .sql("UPDATE timesheet_notification_state "
                 "SET is_locked = FALSE, is_restored = TRUE "
                 "WHERE user_id = ? AND week_start_date = ?")
            .bind(userId)
            .bind(weekStartDate)
            .execute();
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to restore timesheet access: {}", e.what());
        return false;
    }
}

bool NotificationRepository::isTimesheetAccessLocked(int userId)
{
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT COUNT(1) FROM timesheet_notification_state "
                               "WHERE user_id = ? AND is_locked = TRUE AND is_restored = FALSE")
                          .bind(userId)
                          .execute();

        if (auto row = result.fetchOne())
        {
            return row[0].get<int>() > 0;
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to check timesheet lock status: {}", e.what());
    }
    return false;
}
