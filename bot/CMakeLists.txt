cmake_minimum_required(VERSION 3.1)
project(butler CXX)

add_executable(bot main.cpp create_fixed_alarm.cpp create_fixed_alarm.h)
target_link_libraries(bot PRIVATE telegram database)
