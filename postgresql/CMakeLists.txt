cmake_minimum_required(VERSION 3.1)
project(butler CXX)

file(GLOB SOURCES
        src/postgresql.cpp
        src/table_user.cpp
        src/table_alarm.cpp
        src/database.cpp
        )
file(GLOB HEADERS
        include/db/postgresql.h
        include/db/table_user.h
        include/db/table_alarm.h
        include/db/database.h
        )

find_package(libpqxx REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)


add_library(postgresql ${SOURCES} ${HEADERS})
target_link_libraries(postgresql PUBLIC libpqxx::libpqxx spdlog::spdlog fmt::fmt)
target_include_directories(
        postgresql PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Add executable too
add_subdirectory(executable)
