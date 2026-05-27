# examples/simple

add_executable(simple main.cpp)
target_link_libraries(simple PRIVATE PacketForge)
target_include_directories(simple PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})