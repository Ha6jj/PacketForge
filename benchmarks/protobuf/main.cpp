#include "BenchmarkUtils/ISerializer.hpp"
#include "BenchmarkUtils/BenchmarkRunner.hpp"
#include "BenchmarkUtils/CsvReporter.hpp"
#include "benchmark.pb.h"

class ProtobufSerializer : public ISerializer {
public:
    std::string GetName() const override { return "protobuf"; }
    
    std::string Serialize(const BenchmarkData& data) override {
        BenchmarkPayload payload;
        FillPayload(data, payload);
        
        std::string output;
        payload.SerializeToString(&output);
        return output;
    }
    
    BenchmarkData Deserialize(const std::string& serialized) override {
        BenchmarkPayload payload;
        payload.ParseFromString(serialized);
        
        BenchmarkData data;
        data.values.reserve(payload.values_size());
        for (double value : payload.values()) {
            data.values.push_back(value);
        }
        
        for (const auto& kv : payload.metadata()) {
            data.metadata[kv.first] = kv.second;
        }
        
        data.flag = payload.flag();
        data.timestamp = payload.timestamp();
        data.blob = payload.blob();
        
        return data;
    }
    
    size_t GetSerializedSize(const BenchmarkData& data) override {
        BenchmarkPayload payload;
        FillPayload(data, payload);
        return payload.ByteSizeLong();
    }

private:
    void FillPayload(const BenchmarkData& data, BenchmarkPayload& payload) {
        payload.mutable_values()->Reserve(data.values.size());
        for (double value : data.values) {
            payload.add_values(value);
        }
        
        auto* proto_metadata = payload.mutable_metadata();
        for (const auto& kv : data.metadata) {
            (*proto_metadata)[kv.first] = kv.second;
        }
        
        payload.set_flag(data.flag);
        payload.set_timestamp(data.timestamp);
        payload.set_blob(data.blob);
    }
};

int main(int argc, char** argv) {
    BenchmarkRunner<ProtobufSerializer>::RegisterBenchmarks();
    
    benchmark::Initialize(&argc, argv);
    
    CsvReporter reporter("protobuf_benchmark_results.csv");
    benchmark::RunSpecifiedBenchmarks(&reporter);
    
    return 0;
}