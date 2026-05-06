# tests

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(packet_forge_tests
    test_command_factory.cpp
    test_deserializer_header_repository.cpp
    test_packet.cpp
    test_shared_buffer_pool.cpp
    test_vector_view.cpp
)

target_link_libraries(packet_forge_tests PRIVATE GTest::gtest_main PacketForge)

add_test(NAME PacketForgeTests COMMAND packet_forge_tests)

if(ENABLE_COVERAGE)
  target_compile_options(packet_forge_tests PRIVATE --coverage -O0 -g)
  target_link_options(packet_forge_tests PRIVATE --coverage)
endif()