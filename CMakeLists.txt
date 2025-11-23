cmake_minimum_required(VERSION 3.10)
project(PacketForge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(PacketForge INTERFACE)
target_include_directories(PacketForge INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

add_subdirectory(examples)