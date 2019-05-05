cmake_minimum_required(VERSION 3.1)
project(butler CXX)

find_package(libmpdclient REQUIRED)
find_package(spdlog REQUIRED)

add_library(mpd src/mpd_telegram.cpp include/mpd/mpd_telegram.h src/telegram_handler.cpp src/telegram_handler.h src/wrapper.h src/wrapper.cpp)
target_link_libraries(mpd PUBLIC libmpdclient::libmpdclient spdlog::spdlog telegram)
target_include_directories(
        mpd PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Add executable too
# add_subdirectory(executable)
