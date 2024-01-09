cmake_minimum_required(VERSION 3.20)
project(cavlib)
set(CMAKE_CXX_STANDARD 20)

#Emitted compile_commands.json otherwise uses -I flag for includes, which somehow does not work with clangd...
set(MANUAL_INCLUDE "--include-directory=${CMAKE_CURRENT_SOURCE_DIR}/include/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MANUAL_INCLUDE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${MANUAL_INCLUDE} -DCOMP_TESTS")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MANUAL_INCLUDE}")
