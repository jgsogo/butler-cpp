cmake_minimum_required(VERSION 3.1)
project(butler CXX)

file(GLOB SOURCES
        src/user.cpp
        src/alarm_fixed.cpp
        src/alarm_pattern.cpp
        src/database.cpp
        )
file(GLOB HEADERS
        include/db/user.h
        include/db/alarm_fixed.h
        include/db/alarm_pattern.h
        include/db/database.h
        )

find_package(libpqxx REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)


add_library(database ${SOURCES} ${HEADERS})
target_link_libraries(database PUBLIC libpqxx::libpqxx spdlog::spdlog fmt::fmt)
target_include_directories(
        database PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Add executable too
add_subdirectory(executable)
