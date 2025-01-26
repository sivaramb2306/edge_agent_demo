cmake_minimum_required(VERSION 3.10)
project(edge_agent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build tests" OFF)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PISTACHE REQUIRED libpistache)
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    include(GoogleTest)
endif()
find_package(nlohmann_json REQUIRED)

# Add NetSNMP
find_library(NETSNMP_LIBRARY netsnmp)
find_path(NETSNMP_INCLUDE_DIR net-snmp/net-snmp-config.h)

# Main executable
add_executable(rest_server 
    main.cpp
)

target_include_directories(rest_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NETSNMP_INCLUDE_DIR}
)

target_link_libraries(rest_server
    ${PISTACHE_LIBRARIES}
    ${NETSNMP_LIBRARY}
    nlohmann_json::nlohmann_json
)

# Tests
if(BUILD_TESTS)
    enable_testing()

    # Test executable
    add_executable(snmp_client_test
        tests/snmp_client_test.cpp
    )

    target_include_directories(snmp_client_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${NETSNMP_INCLUDE_DIR}
        /usr/include
    )

    target_link_libraries(snmp_client_test
        gtest
        gtest_main
        gmock
        gmock_main
        ${NETSNMP_LIBRARY}
    )

    add_test(NAME snmp_client_test COMMAND snmp_client_test)
endif()
