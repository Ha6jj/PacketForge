#pragma once
#include "benchmark_generated.h"
#include <random>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>

namespace fb_generators {

// --- Вспомогательные функции ---
inline void AddKeyValueDouble(flatbuffers::FlatBufferBuilder& fbb,
                              const std::string& key, double value,
                              std::vector<flatbuffers::Offset<benchmark_fb::KeyValueDouble>>& vec) {
    auto k = fbb.CreateString(key);
    benchmark_fb::KeyValueDoubleBuilder kvb(fbb);
    kvb.add_key(k);
    kvb.add_value(value);
    vec.push_back(kvb.Finish());
}

inline void AddKeyValueString(flatbuffers::FlatBufferBuilder& fbb,
                              const std::string& key, const std::string& value,
                              std::vector<flatbuffers::Offset<benchmark_fb::KeyValueString>>& vec) {
    auto k = fbb.CreateString(key);
    auto v = fbb.CreateString(value);
    benchmark_fb::KeyValueStringBuilder kvb(fbb);
    kvb.add_key(k);
    kvb.add_value(v);
    vec.push_back(kvb.Finish());
}

// --- SystemMetrics ---
struct MetricsGenerator {
    static flatbuffers::unique_ptr<uint8_t[]> Generate(std::size_t, std::size_t) {
        flatbuffers::FlatBufferBuilder fbb(1024);
        
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 100.0);

        // Заполнение custom_metrics (vector вместо map)
        std::vector<flatbuffers::Offset<benchmark_fb::KeyValueDouble>> metrics_vec;
        AddKeyValueDouble(fbb, "load_1m", dist(gen), metrics_vec);
        AddKeyValueDouble(fbb, "load_5m", dist(gen), metrics_vec);
        auto metrics = fbb.CreateVector(metrics_vec);

        auto hostname = fbb.CreateString("bench-node");
        auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        benchmark_fb::SystemMetricsBuilder builder(fbb);
        builder.add_cpu_percent(dist(gen));
        builder.add_memory_percent(20.0 + dist(gen) * 0.75);
        builder.add_disk_io_mbps(dist(gen) * 5.0);
        builder.add_network_rx_mbps(dist(gen) * 10.0);
        builder.add_network_tx_mbps(dist(gen) * 10.0);
        builder.add_custom_metrics(metrics);
        builder.add_epoch_seconds(epoch);
        builder.add_hostname(hostname);
        
        auto root = builder.Finish();
        fbb.Finish(root);
        return fbb.Release();
    }
};

// --- NetworkPacket ---
struct NetworkPacketGenerator {
    static flatbuffers::unique_ptr<uint8_t[]> Generate(std::size_t, std::size_t payload_size) {
        flatbuffers::FlatBufferBuilder fbb(2048 + payload_size);
        
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<uint8_t> byte_dist(0, 255);

        // Генерация payload
        std::vector<uint8_t> payload(payload_size);
        for (auto& b : payload) b = byte_dist(gen);
        auto payload_vec = fbb.CreateVector(payload);

        auto src_ip = fbb.CreateString("192.168.1." + std::to_string(gen() % 255));
        auto dst_ip = fbb.CreateString("10.0.0." + std::to_string(gen() % 255));

        benchmark_fb::NetworkPacketBuilder builder(fbb);
        builder.add_sequence_number(gen());
        builder.add_src_ip(src_ip);
        builder.add_dst_ip(dst_ip);
        builder.add_src_port(1024 + (gen() % 64512));
        builder.add_dst_port(1024 + (gen() % 64512));
        builder.add_payload(payload_vec);
        builder.add_is_encrypted(gen() % 2 == 0);
        builder.add_simulated_latency_ms(0.1 + (gen() % 500) / 10.0);
        
        auto root = builder.Finish();
        fbb.Finish(root);
        return fbb.Release();
    }
};

// --- StrangeBenchmarkData ---
struct StrangeDataGenerator {
    static flatbuffers::unique_ptr<uint8_t[]> Generate(std::size_t num_values, std::size_t blob_size) {
        flatbuffers::FlatBufferBuilder fbb(4096 + num_values * 8 + blob_size);
        
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // values: vector<double>
        std::vector<double> values(num_values);
        for (auto& v : values) v = dist(gen);
        auto values_vec = fbb.CreateVector(values);

        // metadata: vector<KeyValueString>
        std::vector<flatbuffers::Offset<benchmark_fb::KeyValueString>> meta_vec;
        AddKeyValueString(fbb, "source", "benchmark", meta_vec);
        AddKeyValueString(fbb, "version", "1.0", meta_vec);
        AddKeyValueString(fbb, "items_count", std::to_string(num_values), meta_vec);
        auto metadata = fbb.CreateVector(meta_vec);

        // blob: vector<ubyte>
        std::vector<uint8_t> blob(blob_size);
        for (auto& b : blob) b = static_cast<uint8_t>(gen() % 256);
        auto blob_vec = fbb.CreateVector(blob);

        benchmark_fb::StrangeBenchmarkDataBuilder builder(fbb);
        builder.add_values(values_vec);
        builder.add_metadata(metadata);
        builder.add_flag(gen() % 2 == 0);
        builder.add_timestamp(std::time(nullptr));
        builder.add_blob(blob_vec);
        
        auto root = builder.Finish();
        fbb.Finish(root);
        return fbb.Release();
    }
};

// --- TimeSeriesData ---
struct TimeSeriesGenerator {
    static flatbuffers::unique_ptr<uint8_t[]> Generate(std::size_t num_points, std::size_t) {
        flatbuffers::FlatBufferBuilder fbb(4096 + num_points * 12);  // 8+4 байта на точку
        
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> dist(-50.0f, 50.0f);

        // timestamps_ms: vector<long>
        std::vector<int64_t> timestamps(num_points);
        // measurements: vector<float>
        std::vector<float> measurements(num_points);
        
        double interval = 1000.0 / 1000.0;  // sample_rate_hz = 1000
        double current_time = 0.0;
        for (std::size_t i = 0; i < num_points; ++i) {
            timestamps[i] = static_cast<int64_t>(current_time);
            measurements[i] = dist(gen);
            current_time += interval;
        }
        
        auto ts_vec = fbb.CreateVector(timestamps);
        auto meas_vec = fbb.CreateVector(measurements);
        auto sensor_id = fbb.CreateString("TS_" + std::to_string(gen() % 9999));

        benchmark_fb::TimeSeriesDataBuilder builder(fbb);
        builder.add_timestamps_ms(ts_vec);
        builder.add_measurements(meas_vec);
        builder.add_sensor_id(sensor_id);
        builder.add_sample_rate_hz(1000.0);
        builder.add_is_calibrated(true);
        
        auto root = builder.Finish();
        fbb.Finish(root);
        return fbb.Release();
    }
};

} // namespace fb_generators