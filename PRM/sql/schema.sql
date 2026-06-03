CREATE DATABASE IF NOT EXISTS resource_management;
USE resource_management;

CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role ENUM(
        'ADMIN',
        'MANAGER',
        'EMPLOYEE'
    ) NOT NULL,
    status ENUM(
        'ACTIVE',
        'INACTIVE'
    ) NOT NULL DEFAULT 'ACTIVE',
    force_password_change BOOLEAN NOT NULL DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        ON UPDATE CURRENT_TIMESTAMP
);

CREATE TABLE employees (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT UNIQUE,
    full_name VARCHAR(255) NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    department VARCHAR(100),
    designation VARCHAR(100),
    status ENUM(
        'BENCH',
        'ALLOCATED'
    ) DEFAULT 'BENCH',
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_employee_user
        FOREIGN KEY (user_id)
        REFERENCES users(id)
        ON DELETE SET NULL
);

CREATE TABLE skills (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE,
    category ENUM(
        'BACKEND',
        'FRONTEND',
        'DEVOPS',
        'QA',
        'OTHER'
    ) DEFAULT 'OTHER'
);

CREATE TABLE employee_skills (
    employee_id INT NOT NULL,
    skill_id INT NOT NULL,
    proficiency_level ENUM(
        'BEGINNER',
        'INTERMEDIATE',
        'ADVANCED'
    ) NOT NULL,
    PRIMARY KEY (
        employee_id,
        skill_id
    ),
    CONSTRAINT fk_emp_skill_employee
        FOREIGN KEY (employee_id)
        REFERENCES employees(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_emp_skill_skill
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
    status ENUM(
        'PLANNED',
        'ACTIVE',
        'ON_HOLD'
    ) DEFAULT 'PLANNED',
    manager_id INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
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
    status ENUM(
        'NOT_STARTED',
        'IN_PROGRESS',
        'DONE'
    ) DEFAULT 'NOT_STARTED',
    CONSTRAINT fk_milestone_project
        FOREIGN KEY (project_id)
        REFERENCES projects(id)
        ON DELETE CASCADE
);

CREATE TABLE allocations (
    id INT AUTO_INCREMENT PRIMARY KEY,
    employee_id INT NOT NULL,
    project_id INT NOT NULL,
    utilization_percentage INT NOT NULL,
    from_date DATE NOT NULL,
    to_date DATE,
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_allocation_employee
        FOREIGN KEY (employee_id)
        REFERENCES employees(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_allocation_project
        FOREIGN KEY (project_id)
        REFERENCES projects(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_allocation_creator
        FOREIGN KEY (created_by)
        REFERENCES users(id)
        ON DELETE SET NULL
);

CREATE TABLE timesheets (
    id INT AUTO_INCREMENT PRIMARY KEY,
    employee_id INT NOT NULL,
    week_start_date DATE NOT NULL,
    submitted_at TIMESTAMP NULL,
    status ENUM(
        'SUBMITTED',
        'MISSED'
    ) DEFAULT 'MISSED',
    CONSTRAINT fk_timesheet_employee
        FOREIGN KEY (employee_id)
        REFERENCES employees(id)
        ON DELETE CASCADE
);

CREATE TABLE timesheet_lines (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timesheet_id INT NOT NULL,
    project_id INT NOT NULL,
    hours_worked INT NOT NULL,
    CONSTRAINT fk_timesheet_line_timesheet
        FOREIGN KEY (timesheet_id)
        REFERENCES timesheets(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_timesheet_line_project
        FOREIGN KEY (project_id)
        REFERENCES projects(id)
        ON DELETE CASCADE
);

CREATE TABLE timesheet_line_tags (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timesheet_line_id INT NOT NULL,
    activity_tag VARCHAR(100) NOT NULL,
    CONSTRAINT fk_tag_line
        FOREIGN KEY (timesheet_line_id)
        REFERENCES timesheet_lines(id)
        ON DELETE CASCADE
);