cmake_minimum_required(VERSION 3.1)
project(butler CXX)

add_executable(database_test main.cpp)
target_link_libraries(database_test PRIVATE database)
