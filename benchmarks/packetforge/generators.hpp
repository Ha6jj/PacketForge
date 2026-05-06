#pragma once

#include "schemas.hpp"

#include <string>
#include <map>
#include <cstdint>
#include <vector>
#include <random>
#include <chrono>
#include <ctime>

namespace packetforge_generators {

struct MetricsGenerator {
    static SystemMetrics Generate(std::size_t, std::size_t) {
        SystemMetrics m;
        m.hostname = "bench-node";
        m.epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        m.cpu_percent = dist(gen);
        m.memory_percent = 20.0 + (dist(gen) * 0.75);
        m.disk_io_mbps = dist(gen) * 5.0;
        m.network_rx_mbps = dist(gen) * 10.0;
        m.network_tx_mbps = dist(gen) * 10.0;

        m.custom_metrics["load_1m"] = dist(gen);
        m.custom_metrics["load_5m"] = dist(gen);

        return m;
    }
};

struct NetworkPacketGenerator {
    static NetworkPacket Generate(std::size_t, std::size_t payload_size) {
        NetworkPacket pkt;
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<uint8_t> byte_dist(0, 255);

        pkt.sequence_number = gen();
        pkt.src_ip = "192.168.1." + std::to_string(gen() % 255);
        pkt.dst_ip = "10.0.0." + std::to_string(gen() % 255);
        pkt.src_port = 1024 + (gen() % 64512);
        pkt.dst_port = 1024 + (gen() % 64512);
        
        pkt.payload.resize(payload_size);
        for (auto& b : pkt.payload) b = byte_dist(gen);
        
        pkt.is_encrypted = (gen() % 2 == 0);
        pkt.simulated_latency_ms = 0.1 + (gen() % 500) / 10.0;
        return pkt;
    }
};

class StrangeDataGenerator {
public:
    static StrangeBenchmarkData Generate(std::size_t num_values, std::size_t blob_size) {
        StrangeBenchmarkData data;
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<> dist(0.0, 1.0);
        
        data.values.reserve(num_values);
        for (size_t i = 0; i < num_values; ++i) {
            data.values.push_back(dist(gen));
        }

        data.metadata["source"] = "benchmark";
        data.metadata["version"] = "1.0";
        data.metadata["items_count"] = std::to_string(num_values);
        
        data.flag = (gen() % 2 == 0);
        data.timestamp = std::time(nullptr);
        data.blob = std::string(blob_size, static_cast<char>(gen() % 256));
        
        return data;
    }
};

struct TimeSeriesGenerator {
    static TimeSeriesData Generate(std::size_t num_points, std::size_t) {
        TimeSeriesData data;
        data.sample_rate_hz = 1000.0;
        data.sensor_id = "TS_" + std::to_string(std::random_device{}() % 9999);
        data.is_calibrated = true;

        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> dist(-50.0f, 50.0f);

        data.timestamps_ms.reserve(num_points);
        data.measurements.reserve(num_points);

        double interval = 1000.0 / data.sample_rate_hz;
        double current_time = 0.0;
        for (std::size_t i = 0; i < num_points; ++i) {
            data.timestamps_ms.push_back(static_cast<int64_t>(current_time));
            data.measurements.push_back(dist(gen));
            current_time += interval;
        }
        return data;
    }
};

} // namespace packetforge_generators