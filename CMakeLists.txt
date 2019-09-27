cmake_minimum_required(VERSION 3.13)
project(so5)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS system thread filesystem program_options REQUIRED)
find_package(Threads REQUIRED)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_VERSION: ${Boost_VERSION}")

add_executable(runTunProxy
        tun_adapter.cpp
        main.cpp
        proxy.cpp
        cli_handler.cpp
        proto_parser.cpp)
target_link_libraries(runTunProxy ${Boost_LIBRARIES})

add_executable(runTests
        tests/tests.cpp
        tun_adapter.cpp
        proxy.cpp
        cli_handler.cpp
        proto_parser.cpp)

target_link_libraries(runTests
        ${Boost_LIBRARIES}
        ${GTEST_LIBRARIES}
        pthread)

add_executable(runSx5Proxy
        boost_socks5.cpp
        )

target_link_libraries(runSx5Proxy
        ${Boost_LIBRARIES}
        pthread)