cmake_minimum_required(VERSION 3.13)

project(tunproxy)

set(PROJECT_TESTS "tunproxyTests")
set(SOCKS5_PROXY "socks5proxy")
set(TCP_SERVER "tcpserver")

SET(PROJECT_SRCS
        tun_adapter.cpp
        proxy.cpp
        cli_handler.cpp
        proto_parser.cpp)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS system thread filesystem program_options REQUIRED)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_VERSION: ${Boost_VERSION}")


add_executable(${PROJECT_NAME} main.cpp ${PROJECT_SRCS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

add_executable(${PROJECT_TESTS} tests/tests.cpp  ${PROJECT_SRCS})
target_link_libraries(${PROJECT_TESTS} ${Boost_LIBRARIES} ${GTEST_LIBRARIES})

add_executable(${SOCKS5_PROXY} boost_socks5.cpp)
target_link_libraries(${SOCKS5_PROXY} ${Boost_LIBRARIES})

add_executable(${TCP_SERVER} tcp_echo_server.cpp)
target_link_libraries(${TCP_SERVER} ${Boost_LIBRARIES})
