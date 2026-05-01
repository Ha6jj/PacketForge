# benchmarks/include

add_library(BenchmarkUtils INTERFACE)
target_include_directories(BenchmarkUtils INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)