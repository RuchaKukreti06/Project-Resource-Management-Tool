cmake_minimum_required(VERSION 3.21)
project(prm_server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "If using vcpkg, configure CMake with -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(mysql-concpp CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_library(prm-server-lib STATIC
    src/app/Application.cpp
    src/database/Database.cpp
    src/utils/ConfigLoader.cpp
    src/services/AuthService.cpp
    src/repositories/UserRepository.cpp
)

target_include_directories(prm-server-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app  
    ${CMAKE_CURRENT_SOURCE_DIR}/include/database
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/include/services
    ${CMAKE_CURRENT_SOURCE_DIR}/include/repositories
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/models
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
)

target_link_libraries(prm-server-lib PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    mysql::concpp-xdevapi
    jwt-cpp::jwt-cpp
    OpenSSL::Crypto
    httplib::httplib
)

add_executable(prm-server
    src/main.cpp
)

target_link_libraries(prm-server PRIVATE prm-server-lib)

add_executable(auth-service-tests
    tests/AuthServiceTests.cpp
)

target_link_libraries(auth-service-tests PRIVATE
    prm-server-lib
    GTest::gtest
    GTest::gtest_main
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(auth-service-tests)