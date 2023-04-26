cmake_minimum_required(VERSION 3.24)
project(uhd C)

set(CMAKE_C_STANDARD 11)

find_package(UHD "3.7.1" REQUIRED)

include_directories(${UHD_INCLUDE_DIRS})

add_executable(uhd main.c)

target_link_libraries(uhd ${UHD_LIBRARIES} -lm)
