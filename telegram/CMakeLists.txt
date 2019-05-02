cmake_minimum_required(VERSION 3.1)
project(butler CXX)

file(GLOB SOURCES
        src/bot.cpp
        src/alarm.cpp
        src/datetime.cpp)
file(GLOB HEADERS
        include/telegram/bot.h
        include/telegram/alarm.h
        include/telegram/datetime.h)

find_package(tgbot_cpp REQUIRED)
find_package(spdlog REQUIRED)


add_library(telegram ${SOURCES} ${HEADERS})
target_link_libraries(telegram PUBLIC tgbot_cpp::tgbot_cpp spdlog::spdlog)
target_include_directories(
        telegram PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Add executable too
add_subdirectory(executable)
