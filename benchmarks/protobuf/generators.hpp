#pragma once
#include <random>
#include <chrono>
#include "benchmark.pb.h"

namespace benchmark_generators {

// --- SystemMetrics ---
struct MetricsGenerator {
    static benchmark_proto::SystemMetrics Generate(std::size_t, std::size_t) {
        benchmark_proto::SystemMetrics m;
        m.set_hostname("bench-node");
        m.set_epoch_seconds(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 100.0);

        m.set_cpu_percent(dist(gen));
        m.set_memory_percent(20.0 + dist(gen) * 0.75);
        m.set_disk_io_mbps(dist(gen) * 5.0);
        m.set_network_rx_mbps(dist(gen) * 10.0);
        m.set_network_tx_mbps(dist(gen) * 10.0);

        // Заполнение map
        auto* cm = m.mutable_custom_metrics();
        (*cm)["load_1m"] = dist(gen);
        (*cm)["load_5m"] = dist(gen);
        return m;
    }
};

// --- StrangeBenchmarkData ---
struct StrangeDataGenerator {
    static benchmark_proto::StrangeBenchmarkData Generate(std::size_t num_values, std::size_t blob_size) {
        benchmark_proto::StrangeBenchmarkData data;
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        data.mutable_values()->Reserve(num_values);
        for (size_t i = 0; i < num_values; ++i) {
            data.add_values(dist(gen));
        }

        auto* meta = data.mutable_metadata();
        (*meta)["source"] = "benchmark";
        (*meta)["version"] = "1.0";
        (*meta)["items_count"] = std::to_string(num_values);

        data.set_flag(gen() % 2 == 0);
        data.set_timestamp(std::time(nullptr));
        
        // Запись бинарных данных в bytes
        const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::uniform_int_distribution<std::size_t> char_dist(0, charset.size() - 1);
        
        std::string blob(blob_size, '\0');
        for (auto& c : blob) {
            c = charset[char_dist(gen) % 62];
        }
        data.set_blob(std::move(blob));

        return data;
    }
};

// --- TimeSeriesData ---
struct TimeSeriesGenerator {
    static benchmark_proto::TimeSeriesData Generate(std::size_t num_points, std::size_t) {
        benchmark_proto::TimeSeriesData data;
        data.set_sample_rate_hz(1000.0);
        data.set_sensor_id("TS_" + std::to_string(std::random_device{}() % 9999));
        data.set_is_calibrated(true);

        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> dist(-50.0f, 50.0f);

        data.mutable_timestamps_ms()->Reserve(num_points);
        data.mutable_measurements()->Reserve(num_points);

        double interval = 1000.0 / data.sample_rate_hz();
        double current_time = 0.0;
        for (std::size_t i = 0; i < num_points; ++i) {
            data.add_timestamps_ms(static_cast<int64_t>(current_time));
            data.add_measurements(dist(gen));
            current_time += interval;
        }
        return data;
    }
};

// --- NetworkPacket ---
struct NetworkPacketGenerator {
    static benchmark_proto::NetworkPacket Generate(std::size_t, std::size_t payload_size) {
        benchmark_proto::NetworkPacket pkt;
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<uint8_t> byte_dist(0, 255);

        pkt.set_sequence_number(gen());
        pkt.set_src_ip("192.168.1." + std::to_string(gen() % 255));
        pkt.set_dst_ip("10.0.0." + std::to_string(gen() % 255));
        pkt.set_src_port(1024 + (gen() % 64512));
        pkt.set_dst_port(1024 + (gen() % 64512));

        // Генерация payload в bytes
        std::string payload(payload_size, '\0');
        for (auto& b : payload) b = static_cast<char>(byte_dist(gen));
        pkt.set_payload(payload);

        pkt.set_is_encrypted(gen() % 2 == 0);
        pkt.set_simulated_latency_ms(0.1 + (gen() % 500) / 10.0);
        return pkt;
    }
};

} // namespace benchmark_generators