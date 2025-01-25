cmake_minimum_required(VERSION 3.10)
project(edge_agent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PISTACHE REQUIRED libpistache)
find_package(GTest REQUIRED COMPONENTS GTest GMock)
find_package(nlohmann_json REQUIRED)

# Add NetSNMP
find_library(NETSNMP_LIBRARY netsnmp)
find_path(NETSNMP_INCLUDE_DIR net-snmp/net-snmp-config.h)

# Main executable
add_executable(edge_agent 
    main.cpp
)

target_include_directories(edge_agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NETSNMP_INCLUDE_DIR}
)

target_link_libraries(edge_agent
    ${PISTACHE_LIBRARIES}
    ${NETSNMP_LIBRARY}
    nlohmann_json::nlohmann_json
)

# Tests
enable_testing()

# Test executable
add_executable(snmp_client_test
    tests/snmp_client_test.cpp
)

target_include_directories(snmp_client_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NETSNMP_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(snmp_client_test
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    ${NETSNMP_LIBRARY}
)

add_test(NAME snmp_client_test COMMAND snmp_client_test)
