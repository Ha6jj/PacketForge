# benchmarks/packetforge

add_executable(packetforge_benchmark main.cpp)
target_link_libraries(packetforge_benchmark PRIVATE
    benchmark::benchmark
    BenchmarkUtils
    PacketForge
)

target_compile_options(packetforge_benchmark PRIVATE
    -O3
    -march=native
    -flto
)