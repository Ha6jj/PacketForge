#pragma once

#include "BenchmarkUtils/ISerializer.hpp"

#include "PacketForge/macros/PacketStructure.hpp"
#include "PacketForge/macros/CommandSuit.hpp"
#include "PacketForge/serializers/IntSerializers.hpp"
#include "PacketForge/serializers/StringSerializers.hpp"
#include "PacketForge/serializers/VectorSerializers.hpp"
#include "PacketForge/serializers/MapSerializers.hpp"

struct Config : packet_forge::DefaultConfig {
    static constexpr bool use_buffer_pool = true;
};

DEFINE_COMMAND_SUIT(BenchmarkSuit, Config,
    MetricsCommand,
    NetworkCommand,
    StrangeBenchmarkCommand,
    TimeSeriesCommand
)

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

PACKET_STRUCTURE(SystemMetrics,
    &SystemMetrics::cpu_percent,
    &SystemMetrics::memory_percent,
    &SystemMetrics::disk_io_mbps,
    &SystemMetrics::network_rx_mbps,
    &SystemMetrics::network_tx_mbps,
    &SystemMetrics::custom_metrics,
    &SystemMetrics::epoch_seconds,
    &SystemMetrics::hostname
)

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

PACKET_STRUCTURE(NetworkPacket,
    &NetworkPacket::sequence_number,
    &NetworkPacket::src_ip,
    &NetworkPacket::dst_ip,
    &NetworkPacket::src_port,
    &NetworkPacket::dst_port,
    &NetworkPacket::payload,
    &NetworkPacket::is_encrypted,
    &NetworkPacket::simulated_latency_ms
)

struct StrangeBenchmarkData {
    std::vector<double> values;
    std::map<std::string, std::string> metadata;
    bool flag;
    int64_t timestamp;
    std::string blob;
};

PACKET_STRUCTURE(StrangeBenchmarkData,
    &StrangeBenchmarkData::values,
    &StrangeBenchmarkData::metadata,
    &StrangeBenchmarkData::flag,
    &StrangeBenchmarkData::timestamp,
    &StrangeBenchmarkData::blob
)

struct TimeSeriesData {
    std::vector<int64_t> timestamps_ms;
    std::vector<float> measurements;
    std::string sensor_id;
    double sample_rate_hz;
    bool is_calibrated;
};

PACKET_STRUCTURE(TimeSeriesData,
    &TimeSeriesData::timestamps_ms,
    &TimeSeriesData::measurements,
    &TimeSeriesData::sensor_id,
    &TimeSeriesData::sample_rate_hz,
    &TimeSeriesData::is_calibrated
)