find_package(benchmark REQUIRED)

add_library(BenchmarkUtils INTERFACE)
target_include_directories(BenchmarkUtils INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(protobuf)
