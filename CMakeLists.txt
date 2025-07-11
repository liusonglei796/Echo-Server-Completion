cmake_minimum_required(VERSION 3.28)
project(network_test)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(server server.cpp util.cpp)
add_executable(client client.cpp util.cpp)