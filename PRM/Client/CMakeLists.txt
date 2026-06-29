cmake_minimum_required(VERSION 3.21)
project(prm_server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Pass -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Set CMAKE_PREFIX_PATH to include vcpkg_installed directory
if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg_installed/x64-windows")
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_library(client-lib STATIC
    src/api/ApiClient.cpp
    src/services/AuthClientService.cpp
    src/services/UserClientService.cpp
    src/screens/Screen.cpp
    src/services/EmployeeClientService.cpp
    src/services/ProjectClientService.cpp
    src/services/AllocationClientService.cpp
    src/services/TimesheetClientService.cpp
    src/services/AiClientService.cpp
    src/services/ConfigClientService.cpp
    src/utils/ConsoleInput.cpp
    src/utils/DateUtils.cpp
    src/api/AuthSession.cpp
    src/app/Application.cpp
    src/app/Router.cpp
    src/screens/LoginScreen.cpp
    src/screens/ChangePasswordScreen.cpp
    src/screens/ScreenDecorator.cpp
    src/screens/ScreenInput.cpp
    src/screens/ScreenUtils.cpp 
    src/screens/admin/AdminScreen.cpp
    src/screens/admin/AllAllocationsScreen.cpp
    src/screens/admin/ManageUsersScreen.cpp
    src/screens/admin/ManageEmployeesScreen.cpp
    src/screens/admin/ManageProjectsScreen.cpp
    src/screens/admin/SystemConfigScreen.cpp
    src/screens/manager/ManagerScreen.cpp
    src/screens/manager/ResourceDashboardScreen.cpp
    src/screens/manager/AllocateResourceScreen.cpp
    src/screens/manager/MyProjectsScreen.cpp
    src/screens/manager/TimesheetsScreen.cpp
    src/screens/manager/AIAssistantScreen.cpp
    src/screens/employee/EmployeeScreen.cpp
    src/screens/employee/SubmitTimesheetScreen.cpp
)

target_include_directories(client-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app
    ${CMAKE_CURRENT_SOURCE_DIR}/include/screens
    ${CMAKE_CURRENT_SOURCE_DIR}/include/screens/admin
    ${CMAKE_CURRENT_SOURCE_DIR}/include/screens/manager
    ${CMAKE_CURRENT_SOURCE_DIR}/include/screens/employee
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    )

target_link_libraries(client-lib PUBLIC
    nlohmann_json::nlohmann_json
    jwt-cpp::jwt-cpp
    OpenSSL::Crypto
    httplib::httplib
)

add_executable(ClientApp
    src/main.cpp
)

target_link_libraries(ClientApp PRIVATE client-lib)

enable_testing()
include(GoogleTest)
