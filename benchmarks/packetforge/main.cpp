#include "PacketForgeSerializer.hpp"

#include "BenchmarkUtils/BenchmarkRunner.hpp"
#include "BenchmarkUtils/CsvReporter.hpp"

#include <benchmark/benchmark.h>

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    
    BenchmarkRunner<PacketForgeSerializer>::RegisterBenchmarks();
    
    CsvReporter reporter("packetforge_benchmark_results.csv");
    benchmark::RunSpecifiedBenchmarks(&reporter);
    
    return 0;
}