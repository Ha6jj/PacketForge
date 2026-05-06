#pragma once

#include <benchmark/benchmark.h>

#include <string>
#include <vector>

template <typename Serializer, typename DataType, typename Generator>
class BenchmarkRunner {
public:
    static void Register(const std::string& base_name, 
                         const std::vector<std::pair<size_t, size_t>>& args) {
        std::string label = Serializer{}.GetName();

        auto* ser_bench = benchmark::RegisterBenchmark(
            (base_name + "_" + label + "_serialize").data(),
            &BenchmarkRunner::RunSerializeBenchmark
        );
        auto* des_bench = benchmark::RegisterBenchmark(
            (base_name + "_" + label + "_deserialize").data(),
            &BenchmarkRunner::RunDeserializeBenchmark
        );

        for (const auto& [num, blob] : args) {
            ser_bench->Args({static_cast<int64_t>(num), static_cast<int64_t>(blob)});
            des_bench->Args({static_cast<int64_t>(num), static_cast<int64_t>(blob)});
        }

        ser_bench->UseRealTime()->Unit(benchmark::kMicrosecond);
        des_bench->UseRealTime()->Unit(benchmark::kMicrosecond);
    }

private:
    static void RunSerializeBenchmark(benchmark::State& state) {
        Serializer serializer;
        RunSerialize(state, serializer);
    }

    static void RunDeserializeBenchmark(benchmark::State& state) {
        Serializer serializer;
        RunDeserialize(state, serializer);
    }

    static void RunSerialize(benchmark::State& state, Serializer& serializer) {
        const size_t first = static_cast<size_t>(state.range(0));
        const size_t second  = static_cast<size_t>(state.range(1));

        DataType data = Generator::Generate(first, second);
        const size_t serialized_size = serializer.GetSerializedSize(data);

        for (auto _ : state) {
            auto output = serializer.Serialize(data);
            benchmark::DoNotOptimize(output);
            benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * serialized_size);
        state.SetItemsProcessed(state.iterations());
        state.counters["payload_size"] = static_cast<double>(serialized_size);
    }

    static void RunDeserialize(benchmark::State& state, Serializer& serializer) {
        const size_t first = static_cast<size_t>(state.range(0));
        const size_t second  = static_cast<size_t>(state.range(1));

        DataType data = Generator::Generate(first, second);
        auto serialized = serializer.Serialize(data);

        for (auto _ : state) {
            DataType result = serializer.Deserialize(serialized);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * serialized.size());
        state.SetItemsProcessed(state.iterations());
        state.counters["payload_size"] = static_cast<double>(serialized.size());
    }
};
