cmake_minimum_required(VERSION 3.14)

project("cavlib"
        VERSION 0.1
        DESCRIPTION "cavlib"
        HOMEPAGE_URL "https://github.com/c4v4/cavlib"
        LANGUAGES CXX)

add_library(cav INTERFACE)
add_library(cav::cav ALIAS cav)

target_compile_features(cav INTERFACE cxx_std_20)

target_include_directories(
    cav INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

