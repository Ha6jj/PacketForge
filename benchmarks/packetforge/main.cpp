#include "DataSerializer.hpp"

#include "BenchmarkUtils/BenchmarkRunner.hpp"
#include "BenchmarkUtils/CsvReporter.hpp"
#include "generators.hpp"

#include <benchmark/benchmark.h>

constexpr char packetforge_name[] = "packetforge";
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
        DataSerializer<
            SystemMetrics,
            packet_forge::CommandType<packet_forge::BenchmarkSuit_tag>::MetricsCommand,
            metrics_name
        >,
        SystemMetrics,
        packetforge_generators::MetricsGenerator
    >::Register(packetforge_name, args);

    BenchmarkRunner<
        DataSerializer<
            NetworkPacket,
            packet_forge::CommandType<packet_forge::BenchmarkSuit_tag>::NetworkCommand,
            network_name
        >,
        NetworkPacket,
        packetforge_generators::NetworkPacketGenerator
    >::Register(packetforge_name, args);

    BenchmarkRunner<
        DataSerializer<
            StrangeBenchmarkData,
            packet_forge::CommandType<packet_forge::BenchmarkSuit_tag>::StrangeBenchmarkCommand,
            strange_name
        >,
        StrangeBenchmarkData,
        packetforge_generators::StrangeDataGenerator
    >::Register(packetforge_name, args);

    BenchmarkRunner<
        DataSerializer<
            TimeSeriesData,
            packet_forge::CommandType<packet_forge::BenchmarkSuit_tag>::TimeSeriesCommand,
            time_series_name
        >,
        TimeSeriesData,
        packetforge_generators::TimeSeriesGenerator
    >::Register(packetforge_name, args);
    
    CsvReporter reporter("packetforge_benchmark_results.csv");
    benchmark::RunSpecifiedBenchmarks(&reporter);
    
    return 0;
}