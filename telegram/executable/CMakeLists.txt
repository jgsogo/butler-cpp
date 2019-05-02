cmake_minimum_required(VERSION 3.1)
project(butler CXX)

add_executable(telegram_bot main.cpp)
target_link_libraries(telegram_bot PRIVATE telegram)
