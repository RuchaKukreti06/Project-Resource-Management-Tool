# Project & Resource Management (PRM) Tool

A comprehensive enterprise solution for managing users, employee profiles, skills, project milestones, timesheets, and capacity allocation, enriched with AI-powered skill matching and project health intelligence.

## Architecture

This project is built using a clean, layered architecture implementing **SOLID** principles:
- **Client**: A robust C++ Command Line Interface application handling UI routing, user input, and formatted data display using the API.
- **Server**: A high-performance C++ HTTP REST Server powered by `cpp-httplib`, managing domain logic, JWT authentication, and MySQL interactions.
- **Shared**: Common Data Transfer Objects (DTOs) and utilities shared between Client and Server.

### Key Design Patterns & Principles
1. **Repository Pattern**: Data access logic is strictly isolated in Repository classes (`UserRepository`, `TimesheetRepository`), making it easy to swap implementations and mock them for testing.
2. **Dependency Injection**: Services and Repositories are injected into Controllers, enabling robust unit testing and separation of concerns.
3. **DRY & Separation of Concerns**: Auth logic is handled entirely by `AuthMiddleware`, freeing controllers from repetitive security checks.

## Setup Instructions

### Prerequisites
- C++17 Compatible Compiler (MSVC, GCC, Clang)
- CMake (>= 3.21)
- VCPKG (for dependency management)
- MySQL Server (>= 8.0)

### 1. Database Setup
1. Create a MySQL database (e.g., `prm_db`).
2. Run the provided seed script to initialize schemas and seed data:
   ```bash
   mysql -u root -p prm_db < PRM/sql/seed.sql
   ```

### 2. Build Instructions
1. Install dependencies using VCPKG.
2. Configure CMake:
   ```bash
   cd PRM
   cmake -B build -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
   ```
3. Build the project:
   ```bash
   cmake --build build --config Release
   ```

### 3. Configuration
Copy `config.example.json` to `config.json` in your server execution directory, and update the MySQL credentials and JWT secret.

## Running the Application

### Start the Server
```bash
./PRM/build/Server/Release/ServerApp.exe
```
The server will start listening on `http://localhost:8080`.

### Start the Client
In a new terminal window:
```bash
./PRM/build/Client/Release/ClientApp.exe
```

## Testing

The project uses **GTest** and **GMock** to achieve >60% unit test coverage across all Services and Controllers. Currently, all tests pass successfully with a 100% pass rate.

To run the test suite:
```bash
cd PRM/build
ctest -C Release --output-on-failure
```

## Documentation
- **API Documentation**: Interactive Swagger/OpenAPI documentation is available in `PRM/Server/docs/openapi.yml`.
- **System Diagrams**: Entity-Relationship, Use Case, and Process Flow diagrams are in `system_diagrams.md`.

## Features
- **Role-Based Access**: Distinct Admin, Manager, and Employee portals.
- **AI Talent Matching**: Intelligently recommends employees for projects based on skills and capacity using Google Gemini LLM.
- **Project Health Scheduler**: Offline background thread that computes allocation limits and warns managers of overdue milestones.
- **Timesheet Enforcement**: Locks employee submissions if timesheets are missed, requiring managerial overrides.