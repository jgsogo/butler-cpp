cmake_minimum_required(VERSION 3.1)
project(butler CXX)

add_executable(bot main.cpp create_alarm.cpp create_alarm.h alarm_utils.h alarm_utils.cpp)
target_link_libraries(bot PRIVATE telegram database mpd)
