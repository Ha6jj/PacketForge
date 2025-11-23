#include <benchmark/benchmark.h>
#include <fstream>
#include <random>
#include "benchmark.pb.h"

// Auxiliary class for test data generation
class PayloadGenerator {
public:
    static BenchmarkPayload CreatePayload(size_t num_values, size_t blob_size) {
        BenchmarkPayload payload;
        
        // Fill repeated values
        payload.mutable_values()->Reserve(num_values);
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<> dist(0.0, 1.0);
        for (size_t i = 0; i < num_values; ++i) {
            payload.add_values(dist(gen));
        }
        
        // Add metadata
        (*payload.mutable_metadata())["source"] = "benchmark";
        (*payload.mutable_metadata())["version"] = "1.0";
        
        // Set other fields
        payload.set_flag(true);
        payload.set_timestamp(std::time(nullptr));
        
        // Generate random blob
        payload.set_blob(std::string(blob_size, 'x'));
        
        return payload;
    }
};

// Auxiliary class for CSV reporting
class CsvReporter : public benchmark::BenchmarkReporter {
public:
    explicit CsvReporter(const std::string& filename) : file_(filename) {
        file_ << "benchmark_name,time_unit,"
            << "iterations,real_time,cpu_time\n";
    }

    bool ReportContext(const Context& context) override { return true; }
    
    void ReportRuns(const std::vector<Run>& reports) override {
        for (const auto& run : reports) {
        file_ << run.benchmark_name() << ","
                << run.time_unit << ","
                << run.iterations << ","
                << run.real_accumulated_time << ","
                << run.cpu_accumulated_time << "\n";
        }
        file_.flush();
    }

    void Finalize() override {}

private:
    std::ofstream file_;
};

// Serialization benchmark
static void BM_Serialize(benchmark::State& state) {
    const size_t num_values = state.range(0);
    const size_t blob_size = state.range(1);
    
    auto payload = PayloadGenerator::CreatePayload(num_values, blob_size);
    std::string output;
    
    // Pre-serialize to get size for bytes processed metric
    payload.SerializeToString(&output);
    const size_t bytes_per_iter = output.size();
    
    for (auto _ : state) {
        output.clear();
        benchmark::DoNotOptimize(payload.SerializeToString(&output));
        benchmark::ClobberMemory();
    }
    
    state.SetBytesProcessed(state.iterations() * bytes_per_iter);
    state.SetItemsProcessed(state.iterations());
}

// Deserialization benchmark
static void BM_Deserialize(benchmark::State& state) {
    const size_t num_values = state.range(0);
    const size_t blob_size = state.range(1);
    
    auto payload = PayloadGenerator::CreatePayload(num_values, blob_size);
    std::string serialized;
    payload.SerializeToString(&serialized);
    
    BenchmarkPayload result;
    for (auto _ : state) {
        benchmark::DoNotOptimize(result.ParseFromString(serialized));
        benchmark::ClobberMemory();
    }
    
    state.SetBytesProcessed(state.iterations() * serialized.size());
    state.SetItemsProcessed(state.iterations());
}

// Register benchmarks with parameter ranges
BENCHMARK(BM_Serialize)
  ->Args({100, 1024})    // Small payload
  ->Args({10000, 4096})  // Medium payload
  ->Args({100000, 16384})// Large payload
  ->UseRealTime();

BENCHMARK(BM_Deserialize)
  ->Args({100, 1024})
  ->Args({10000, 4096})
  ->Args({100000, 16384})
  ->UseRealTime();

int main(int argc, char** argv) {
    // Initialize benchmark framework
    benchmark::Initialize(&argc, argv);
    
    // Custom CSV reporter
    CsvReporter reporter("protobuf_benchmark_results.csv");
    benchmark::RunSpecifiedBenchmarks(&reporter);
    
    return 0;
}