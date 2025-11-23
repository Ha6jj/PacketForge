add_executable(example1 main.cpp)
target_link_libraries(example1 PRIVATE PacketForge)
target_include_directories(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})