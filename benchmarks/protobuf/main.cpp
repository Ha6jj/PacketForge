#include "ProtobufSerializer.hpp"
#include "BenchmarkUtils/BenchmarkRunner.hpp"
#include "BenchmarkUtils/CsvReporter.hpp"
#include "benchmark.pb.h" 
#include "generators.hpp"
#include <benchmark/benchmark.h>

constexpr char protobuf_name[] = "protobuf";
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

    BenchmarkRunner<
        ProtobufSerializer<benchmark_proto::SystemMetrics, metrics_name>,
        benchmark_proto::SystemMetrics,
        benchmark_generators::MetricsGenerator
    >::Register(protobuf_name, args);

    BenchmarkRunner<
        ProtobufSerializer<benchmark_proto::NetworkPacket, network_name>,
        benchmark_proto::NetworkPacket,
        benchmark_generators::NetworkPacketGenerator
    >::Register(protobuf_name, args);

    BenchmarkRunner<
        ProtobufSerializer<benchmark_proto::StrangeBenchmarkData, strange_name>,
        benchmark_proto::StrangeBenchmarkData,
        benchmark_generators::StrangeDataGenerator
    >::Register(protobuf_name, args);

    BenchmarkRunner<
        ProtobufSerializer<benchmark_proto::TimeSeriesData, time_series_name>,
        benchmark_proto::TimeSeriesData,
        benchmark_generators::TimeSeriesGenerator
    >::Register(protobuf_name, args);

    CsvReporter reporter("protobuf_benchmark_results.csv");
    benchmark::RunSpecifiedBenchmarks(&reporter);
    
    return 0;
}