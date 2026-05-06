#pragma once

#include "schemas.hpp"
#include "benchmark_generated.h"
#include "BenchmarkUtils/ISerializer.hpp"

#include <flatbuffers/flatbuffers.h>
#include <string>
#include <vector>
#include <stdexcept>

// ============================================================================
// Конвертеры: native structs ↔ FlatBuffers
// ============================================================================
namespace fb_convert {

// --- Вспомогательные функции для map → vector<KeyValue*> ---
inline void AppendKVDouble(flatbuffers::FlatBufferBuilder& fbb,
                           const std::map<std::string, double>& src,
                           std::vector<flatbuffers::Offset<benchmark_fb::KeyValueDouble>>& dst) {
    for (const auto& [k, v] : src) {
        auto key = fbb.CreateString(k);
        benchmark_fb::KeyValueDoubleBuilder builder(fbb);
        builder.add_key(key);
        builder.add_value(v);
        dst.push_back(builder.Finish());
    }
}

inline void AppendKVString(flatbuffers::FlatBufferBuilder& fbb,
                           const std::map<std::string, std::string>& src,
                           std::vector<flatbuffers::Offset<benchmark_fb::KeyValueString>>& dst) {
    for (const auto& [k, v] : src) {
        auto key = fbb.CreateString(k);
        auto val = fbb.CreateString(v);
        benchmark_fb::KeyValueStringBuilder builder(fbb);
        builder.add_key(key);
        builder.add_value(val);
        dst.push_back(builder.Finish());
    }
}

// ============================================================================
// SystemMetrics
// ============================================================================
inline flatbuffers::Offset<benchmark_fb::SystemMetrics> ToFB(
    flatbuffers::FlatBufferBuilder& fbb, const SystemMetrics& src) {
    
    std::vector<flatbuffers::Offset<benchmark_fb::KeyValueDouble>> metrics_vec;
    AppendKVDouble(fbb, src.custom_metrics, metrics_vec);
    auto metrics = fbb.CreateVector(metrics_vec);
    auto hostname = fbb.CreateString(src.hostname);

    benchmark_fb::SystemMetricsBuilder builder(fbb);
    builder.add_cpu_percent(src.cpu_percent);
    builder.add_memory_percent(src.memory_percent);
    builder.add_disk_io_mbps(src.disk_io_mbps);
    builder.add_network_rx_mbps(src.network_rx_mbps);
    builder.add_network_tx_mbps(src.network_tx_mbps);
    builder.add_custom_metrics(metrics);
    builder.add_epoch_seconds(src.epoch_seconds);
    builder.add_hostname(hostname);
    return builder.Finish();
}

inline SystemMetrics FromFB(const benchmark_fb::SystemMetrics* src) {
    SystemMetrics dst;
    dst.cpu_percent = src->cpu_percent();
    dst.memory_percent = src->memory_percent();
    dst.disk_io_mbps = src->disk_io_mbps();
    dst.network_rx_mbps = src->network_rx_mbps();
    dst.network_tx_mbps = src->network_tx_mbps();
    
    if (src->custom_metrics()) {
        for (const auto* kv : *src->custom_metrics()) {
            dst.custom_metrics[kv->key()->str()] = kv->value();
        }
    }
    dst.epoch_seconds = src->epoch_seconds();
    dst.hostname = src->hostname() ? src->hostname()->str() : "";
    return dst;
}

// ============================================================================
// NetworkPacket
// ============================================================================
inline flatbuffers::Offset<benchmark_fb::NetworkPacket> ToFB(
    flatbuffers::FlatBufferBuilder& fbb, const NetworkPacket& src) {
    
    auto src_ip = fbb.CreateString(src.src_ip);
    auto dst_ip = fbb.CreateString(src.dst_ip);
    auto payload = fbb.CreateVector(src.payload);

    benchmark_fb::NetworkPacketBuilder builder(fbb);
    builder.add_sequence_number(src.sequence_number);
    builder.add_src_ip(src_ip);
    builder.add_dst_ip(dst_ip);
    builder.add_src_port(static_cast<uint16_t>(src.src_port));
    builder.add_dst_port(static_cast<uint16_t>(src.dst_port));
    builder.add_payload(payload);
    builder.add_is_encrypted(src.is_encrypted);
    builder.add_simulated_latency_ms(src.simulated_latency_ms);
    return builder.Finish();
}

inline NetworkPacket FromFB(const benchmark_fb::NetworkPacket* src) {
    NetworkPacket dst;
    dst.sequence_number = src->sequence_number();
    dst.src_ip = src->src_ip() ? src->src_ip()->str() : "";
    dst.dst_ip = src->dst_ip() ? src->dst_ip()->str() : "";
    dst.src_port = src->src_port();
    dst.dst_port = src->dst_port();
    
    if (src->payload()) {
        dst.payload.assign(src->payload()->begin(), src->payload()->end());
    }
    dst.is_encrypted = src->is_encrypted();
    dst.simulated_latency_ms = src->simulated_latency_ms();
    return dst;
}

// ============================================================================
// StrangeBenchmarkData
// ============================================================================
inline flatbuffers::Offset<benchmark_fb::StrangeBenchmarkData> ToFB(
    flatbuffers::FlatBufferBuilder& fbb, const StrangeBenchmarkData& src) {
    
    auto values = fbb.CreateVector(src.values);
    
    std::vector<flatbuffers::Offset<benchmark_fb::KeyValueString>> meta_vec;
    AppendKVString(fbb, src.metadata, meta_vec);
    auto metadata = fbb.CreateVector(meta_vec);
    
    auto blob = fbb.CreateVector(
        reinterpret_cast<const uint8_t*>(src.blob.data()), src.blob.size());

    benchmark_fb::StrangeBenchmarkDataBuilder builder(fbb);
    builder.add_values(values);
    builder.add_metadata(metadata);
    builder.add_flag(src.flag);
    builder.add_timestamp(src.timestamp);
    builder.add_blob(blob);
    return builder.Finish();
}

inline StrangeBenchmarkData FromFB(const benchmark_fb::StrangeBenchmarkData* src) {
    StrangeBenchmarkData dst;
    
    if (src->values()) {
        dst.values.assign(src->values()->begin(), src->values()->end());
    }
    if (src->metadata()) {
        for (const auto* kv : *src->metadata()) {
            dst.metadata[kv->key()->str()] = kv->value()->str();
        }
    }
    dst.flag = src->flag();
    dst.timestamp = src->timestamp();
    if (src->blob()) {
        dst.blob.assign(
            reinterpret_cast<const char*>(src->blob()->data()), 
            src->blob()->size());
    }
    return dst;
}

// ============================================================================
// TimeSeriesData
// ============================================================================
inline flatbuffers::Offset<benchmark_fb::TimeSeriesData> ToFB(
    flatbuffers::FlatBufferBuilder& fbb, const TimeSeriesData& src) {
    
    auto timestamps = fbb.CreateVector(src.timestamps_ms);
    auto measurements = fbb.CreateVector(src.measurements);
    auto sensor_id = fbb.CreateString(src.sensor_id);

    benchmark_fb::TimeSeriesDataBuilder builder(fbb);
    builder.add_timestamps_ms(timestamps);
    builder.add_measurements(measurements);
    builder.add_sensor_id(sensor_id);
    builder.add_sample_rate_hz(src.sample_rate_hz);
    builder.add_is_calibrated(src.is_calibrated);
    return builder.Finish();
}

inline TimeSeriesData FromFB(const benchmark_fb::TimeSeriesData* src) {
    TimeSeriesData dst;
    
    if (src->timestamps_ms()) {
        dst.timestamps_ms.assign(
            src->timestamps_ms()->begin(), src->timestamps_ms()->end());
    }
    if (src->measurements()) {
        dst.measurements.assign(
            src->measurements()->begin(), src->measurements()->end());
    }
    dst.sensor_id = src->sensor_id() ? src->sensor_id()->str() : "";
    dst.sample_rate_hz = src->sample_rate_hz();
    dst.is_calibrated = src->is_calibrated();
    return dst;
}

} // namespace fb_convert

// ============================================================================
// Шаблон сериализатора для всех типов данных
// ============================================================================
template <typename Data, const char* Name, typename FBType>
class FlatBufferSerializer : public ISerializer<Data> {
public:
    std::string GetName() const override { return Name; }

    std::string Serialize(const Data& data) override {
        flatbuffers::FlatBufferBuilder fbb(4096);
        auto root = fb_convert::ToFB(fbb, data);
        fbb.Finish(root);
        auto buf = fbb.GetBufferPointer();
        auto size = fbb.GetSize();
        return std::string(reinterpret_cast<const char*>(buf), size);
    }

    Data Deserialize(const std::string& serialized) override {
        if (serialized.empty()) {
            throw std::runtime_error("FlatBuffers: empty input");
        }
        auto ptr = reinterpret_cast<const uint8_t*>(serialized.data());
        auto root = flatbuffers::GetRoot<FBType>(ptr);

        return fb_convert::FromFB(root);
    }

    size_t GetSerializedSize(const Data& data) override {
        flatbuffers::FlatBufferBuilder fbb(4096);
        auto root = fb_convert::ToFB(fbb, data);
        fbb.Finish(root);
        return fbb.GetSize();
    }
};