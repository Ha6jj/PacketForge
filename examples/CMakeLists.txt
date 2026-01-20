add_library(PacketForge INTERFACE)
target_include_directories(PacketForge INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

add_subdirectory(example1)
