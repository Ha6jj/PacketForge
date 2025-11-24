#pragma once

#include "core/DataGenerator.hpp"

#include <benchmark/benchmark.h>

template <typename Serializer>
class BenchmarkRunner {
public:
    static void RegisterBenchmarks() {
        benchmark::RegisterBenchmark(
            (Serializer().GetName() + "_serialize").data(),
            &BenchmarkRunner<Serializer>::RunSerializeBenchmark
        )
        ->Args({100, 1024})
        ->Args({10000, 4096})
        ->Args({100000, 16384})
        ->UseRealTime()
        ->Unit(benchmark::kMicrosecond);
        
        benchmark::RegisterBenchmark(
            (Serializer().GetName() + "_deserialize").data(),
            &BenchmarkRunner<Serializer>::RunDeserializeBenchmark
        )
        ->Args({100, 1024})
        ->Args({10000, 4096})
        ->Args({100000, 16384})
        ->UseRealTime()
        ->Unit(benchmark::kMicrosecond);
    }

private:
    static void RunSerializeBenchmark(benchmark::State& state) {
        RunSerialize(state, Serializer());
    }

    static void RunDeserializeBenchmark(benchmark::State& state) {
        RunDeserialize(state, Serializer());
    }

    static void RunSerialize(benchmark::State& state, Serializer serializer) {
        const size_t num_values = state.range(0);
        const size_t blob_size = state.range(1);
        
        auto data = DataGenerator::Generate(num_values, blob_size);
        const size_t serialized_size = serializer.GetSerializedSize(data);
        
        for (auto _ : state) {
            std::string output;
            benchmark::DoNotOptimize(output = serializer.Serialize(data));
            benchmark::ClobberMemory();
        }
        
        state.SetBytesProcessed(state.iterations() * serialized_size);
        state.SetItemsProcessed(state.iterations());
        state.counters["payload_size"] = static_cast<double>(serialized_size);
    }

    static void RunDeserialize(benchmark::State& state, Serializer serializer) {
        const size_t num_values = state.range(0);
        const size_t blob_size = state.range(1);
        
        auto data = DataGenerator::Generate(num_values, blob_size);
        std::string serialized = serializer.Serialize(data);
        
        for (auto _ : state) {
            BenchmarkData result;
            benchmark::DoNotOptimize(result = serializer.Deserialize(serialized));
            benchmark::ClobberMemory();
        }
        
        state.SetBytesProcessed(state.iterations() * serialized.size());
        state.SetItemsProcessed(state.iterations());
        state.counters["payload_size"] = static_cast<double>(serialized.size());
    }
};
