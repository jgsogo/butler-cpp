cmake_minimum_required(VERSION 3.1)
project(butler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(telegram)
add_subdirectory(db)
add_subdirectory(mpd)
add_subdirectory(bot)