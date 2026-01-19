add_executable(example2 main.cpp)
target_link_libraries(example2 PRIVATE PacketForge)
target_include_directories(example2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})