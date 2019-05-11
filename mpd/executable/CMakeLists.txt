cmake_minimum_required(VERSION 3.1)
project(butler CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(mpd_example main.cpp)
target_link_libraries(mpd_example PRIVATE mpd)
