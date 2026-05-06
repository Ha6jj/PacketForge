#include "DataSerializer.hpp"
#include "BenchmarkUtils/BenchmarkRunner.hpp"
#include "BenchmarkUtils/CsvReporter.hpp"
#include "generators.hpp"
#include <benchmark/benchmark.h>

// Имена для отчётности
constexpr char fb_name[] = "flatbuffers";
constexpr char metrics_name[] = "metrics";
constexpr char network_name[] = "network";
constexpr char strange_name[] = "strange";
constexpr char time_series_name[] = "timeseries";

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    
    const std::vector<std::pair<size_t, size_t>> args = {
        {100,    1024},
        {10000,  4096},
        {100000, 16384}
    };
    
    // SystemMetrics
    BenchmarkRunner<
        FlatBufferSerializer<
            SystemMetrics,
            fb_name,
            benchmark_fb::SystemMetrics
        >,
        SystemMetrics,
        fb_generators::MetricsGenerator
    >::Register(fb_name, args);

    // NetworkPacket
    BenchmarkRunner<
        FlatBufferSerializer<
            NetworkPacket,
            fb_name,
            benchmark_fb::NetworkPacket
        >,
        NetworkPacket,
        fb_generators::NetworkPacketGenerator
    >::Register(fb_name, args);

    // StrangeBenchmarkData
    BenchmarkRunner<
        FlatBufferSerializer<
            StrangeBenchmarkData,
            fb_name,
            benchmark_fb::StrangeBenchmarkData
        >,
        StrangeBenchmarkData,
        fb_generators::StrangeDataGenerator
    >::Register(fb_name, args);

    // TimeSeriesData
    BenchmarkRunner<
        FlatBufferSerializer<
            TimeSeriesData,
            fb_name,
            benchmark_fb::TimeSeriesData
        >,
        TimeSeriesData,
        fb_generators::TimeSeriesGenerator
    >::Register(fb_name, args);

    CsvReporter reporter("flatbuffers_benchmark_results.csv");
    benchmark::RunSpecifiedBenchmarks(&reporter);
    
    return 0;
}