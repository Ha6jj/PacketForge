#pragma once

#include <vector>
#include <map>
#include <string>

struct SystemMetrics {
    double cpu_percent;
    double memory_percent;
    double disk_io_mbps;
    double network_rx_mbps;
    double network_tx_mbps;
    std::map<std::string, double> custom_metrics;
    int64_t epoch_seconds;
    std::string hostname;
};

struct NetworkPacket {
    uint32_t sequence_number;
    std::string src_ip;
    std::string dst_ip;
    uint32_t src_port;
    uint32_t dst_port;
    std::vector<uint8_t> payload;
    bool is_encrypted;
    double simulated_latency_ms;
};

struct StrangeBenchmarkData {
    std::vector<double> values;
    std::map<std::string, std::string> metadata;
    bool flag;
    int64_t timestamp;
    std::string blob;
};

struct TimeSeriesData {
    std::vector<int64_t> timestamps_ms;
    std::vector<float> measurements;
    std::string sensor_id;
    double sample_rate_hz;
    bool is_calibrated;
};
