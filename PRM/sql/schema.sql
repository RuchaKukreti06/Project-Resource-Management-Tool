CREATE DATABASE IF NOT EXISTS resource_management_database_PRM_Tool;
USE resource_management_database_PRM_Tool;

CREATE TABLE roles (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE
);

CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    role_id INT NOT NULL,
    manager_id INT,
    username VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    full_name VARCHAR(255) NOT NULL,
    department VARCHAR(100),
    designation VARCHAR(100),
    force_password_change BOOLEAN NOT NULL DEFAULT TRUE,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_user_role
        FOREIGN KEY (role_id)
        REFERENCES roles(id),
    CONSTRAINT fk_user_manager
        FOREIGN KEY (manager_id)
        REFERENCES users(id)
        ON DELETE SET NULL
);

CREATE TABLE resources (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT UNIQUE,
    status ENUM('BENCH', 'ALLOCATED') NOT NULL DEFAULT 'BENCH',
    total_utilisation INT NOT NULL DEFAULT 0,
    CONSTRAINT fk_resource_user
        FOREIGN KEY (user_id)
        REFERENCES users(id)
        ON DELETE CASCADE
);

CREATE TABLE skills (
    id INT AUTO_INCREMENT PRIMARY KEY,
    skill_name VARCHAR(150) NOT NULL UNIQUE,
    category ENUM('BACKEND', 'FRONTEND', 'DEVOPS', 'QA', 'OTHER') NOT NULL DEFAULT 'OTHER'
);

CREATE TABLE resource_skills (
    id INT AUTO_INCREMENT PRIMARY KEY,
    resource_id INT NOT NULL,
    skill_id INT NOT NULL,
    proficiency_level ENUM('BEGINNER', 'INTERMEDIATE', 'ADVANCED') NOT NULL,
    CONSTRAINT fk_resource_skill_resource
        FOREIGN KEY (resource_id)
        REFERENCES resources(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_resource_skill_skill
        FOREIGN KEY (skill_id)
        REFERENCES skills(id)
        ON DELETE CASCADE
);

CREATE TABLE projects (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    description TEXT,
    start_date DATE,
    end_date DATE,
    total_story_points INT NOT NULL DEFAULT 0,
    status ENUM('PLANNED', 'ACTIVE', 'ON_HOLD', 'COMPLETED') NOT NULL DEFAULT 'PLANNED',
    health_status ENUM('ON_TRACK', 'ATTENTION', 'AT_RISK') NOT NULL DEFAULT 'ON_TRACK',
    manager_id INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_project_manager
        FOREIGN KEY (manager_id)
        REFERENCES users(id)
        ON DELETE SET NULL
);

CREATE TABLE milestones (
    id INT AUTO_INCREMENT PRIMARY KEY,
    project_id INT NOT NULL,
    title VARCHAR(255) NOT NULL,
    due_date DATE,
    story_points INT NOT NULL DEFAULT 0,
    status ENUM('NOT_STARTED', 'IN_PROGRESS', 'DONE') NOT NULL DEFAULT 'NOT_STARTED',
    health_flag ENUM('NORMAL', 'OVERDUE') NOT NULL DEFAULT 'NORMAL',
    CONSTRAINT fk_milestone_project
        FOREIGN KEY (project_id)
        REFERENCES projects(id)
        ON DELETE CASCADE
);

CREATE TABLE allocations (
    id INT AUTO_INCREMENT PRIMARY KEY,
    resource_id INT NOT NULL,
    project_id INT NOT NULL,
    utilisation_percent INT NOT NULL,
    from_date DATE NOT NULL,
    to_date DATE,
    CONSTRAINT fk_allocation_resource
        FOREIGN KEY (resource_id)
        REFERENCES resources(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_allocation_project
        FOREIGN KEY (project_id)
        REFERENCES projects(id)
        ON DELETE CASCADE
);

CREATE TABLE timesheets (
    id INT AUTO_INCREMENT PRIMARY KEY,
    resource_id INT NOT NULL,
    week_start_date DATE NOT NULL,
    status ENUM('SUBMITTED', 'MISSED') NOT NULL DEFAULT 'MISSED',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_timesheet_resource
        FOREIGN KEY (resource_id)
        REFERENCES resources(id)
        ON DELETE CASCADE
);

CREATE TABLE timesheet_entries (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timesheet_id INT NOT NULL,
    project_id INT NOT NULL,
    hours INT NOT NULL,
    CONSTRAINT fk_timesheet_entry_timesheet
        FOREIGN KEY (timesheet_id)
        REFERENCES timesheets(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_timesheet_entry_project
        FOREIGN KEY (project_id)
        REFERENCES projects(id)
        ON DELETE CASCADE
);

CREATE TABLE activity_tags (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timesheet_entry_id INT NOT NULL,
    tag_name VARCHAR(100) NOT NULL,
    CONSTRAINT fk_activity_tag_entry
        FOREIGN KEY (timesheet_entry_id)
        REFERENCES timesheet_entries(id)
        ON DELETE CASCADE
);

CREATE TABLE system_config (
    id INT AUTO_INCREMENT PRIMARY KEY,
    llm_provider VARCHAR(100) NOT NULL,
    llm_api_key VARCHAR(255) NOT NULL,
    scheduler_interval_hrs INT NOT NULL DEFAULT 24,
    max_weekly_hours INT NOT NULL DEFAULT 40,
    smtp_enabled BOOLEAN NOT NULL DEFAULT FALSE,
    smtp_host VARCHAR(255) NOT NULL DEFAULT '',
    smtp_port INT NOT NULL DEFAULT 587,
    smtp_username VARCHAR(255) NOT NULL DEFAULT '',
    smtp_password VARCHAR(255) NOT NULL DEFAULT '',
    smtp_from_email VARCHAR(255) NOT NULL DEFAULT '',
    smtp_from_name VARCHAR(255) NOT NULL DEFAULT '',
    smtp_use_tls BOOLEAN NOT NULL DEFAULT TRUE
);

CREATE TABLE timesheet_notification_state (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    week_start_date DATE NOT NULL,
    notification_stage INT NOT NULL DEFAULT 0,
    is_locked BOOLEAN NOT NULL DEFAULT FALSE,
    is_restored BOOLEAN NOT NULL DEFAULT FALSE,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT uq_timesheet_notification_state_user_week UNIQUE (user_id, week_start_date),
    CONSTRAINT fk_timesheet_notification_state_user
        FOREIGN KEY (user_id)
        REFERENCES users(id)
        ON DELETE CASCADE
);

INSERT INTO roles (name)
VALUES ('ADMIN'), ('MANAGER'), ('EMPLOYEE');

INSERT INTO users (role_id, manager_id, username, password_hash, email, full_name, department, designation, force_password_change, is_active)
VALUES (1, NULL, 'admin', 'e86f78a8a3caf0b60d8e74e5942aa6d86dc150cd3c03338aef25b7d2d7e3acc7', 'admin@example.com', 'System Administrator', NULL, NULL, TRUE, TRUE);
