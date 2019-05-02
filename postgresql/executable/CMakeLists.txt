cmake_minimum_required(VERSION 3.1)
project(butler CXX)

add_executable(postgresql_test main.cpp)
target_link_libraries(postgresql_test PRIVATE postgresql)
