cmake_minimum_required(VERSION 3.21)
project(prm_server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Pass -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(mysql-concpp CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

find_package(httplib CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# ── Sources ───────────────────────────────────────────────────
file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(REMOVE_ITEM SERVER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# ── Library ───────────────────────────────────────────────────
add_library(prm-server-lib STATIC ${SERVER_SOURCES})

# ── Include every subdirectory under include/ automatically ───
target_include_directories(prm-server-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/models
)

file(GLOB INCLUDE_SUBDIRS LIST_DIRECTORIES true
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*)

foreach(DIR ${INCLUDE_SUBDIRS})
    if(IS_DIRECTORY ${DIR})
        target_include_directories(prm-server-lib PUBLIC ${DIR})
    endif()
endforeach()

# ── Links ─────────────────────────────────────────────────────
target_link_libraries(prm-server-lib PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    mysql::concpp-xdevapi
    jwt-cpp::jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    httplib::httplib
)

target_compile_definitions(prm-server-lib PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)

# ── Server executable ─────────────────────────────────────────
add_executable(prm-server src/main.cpp)
target_link_libraries(prm-server PRIVATE prm-server-lib)

# ── Tests ─────────────────────────────────────────────────────
file(GLOB_RECURSE TEST_SOURCES 
        "tests/*.cpp"
        "tests/mocks/*.cpp")
add_executable(prm-tests ${TEST_SOURCES})
target_link_libraries(prm-tests PRIVATE
    prm-server-lib
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)

target_include_directories(prm-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/mocks
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(prm-tests  DISCOVERY_TIMEOUT 60)
