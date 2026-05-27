# math_server/math_server

add_executable(math_server main.cpp)
target_link_libraries(math_server PRIVATE PacketForge)
target_include_directories(math_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})