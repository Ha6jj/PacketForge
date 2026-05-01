#pragma once

#include "BenchmarkUtils/ISerializer.hpp"

#include "PacketForge/CommandFactory.hpp"
#include "PacketForge/macros/PacketStructure.hpp"
#include "PacketForge/macros/CommandSuit.hpp"
#include "PacketForge/serializers/IntSerializers.hpp"
#include "PacketForge/serializers/StringSerializers.hpp"
#include "PacketForge/serializers/VectorSerializers.hpp"
#include "PacketForge/serializers/MapSerializers.hpp"

#include <vector>
#include <string>
#include <stdexcept>

struct Config : DefaultConfig {
    static constexpr bool use_buffer_pool = true;
};

DEFINE_COMMAND_SUIT(BenchmarkSuit, Config,
    BenchmarkCommand
)

PACKET_STRUCTURE(BenchmarkData,
    &BenchmarkData::values,
    &BenchmarkData::metadata,
    &BenchmarkData::flag,
    &BenchmarkData::timestamp,
    &BenchmarkData::blob
)

class PacketForgeSerializer : public ISerializer {
public:
    std::string GetName() const override { return "packetforge"; }

    std::string Serialize(const BenchmarkData& data) override {
        auto& factory = GetFactory();
        auto packet = factory.create(
            packet_forge::CommandType<packet_forge::BenchmarkSuit_tag>::BenchmarkCommand,
            data
        );

        packet_forge::SharedBuffer raw = packet.build();
        return std::string(raw->data.begin(), raw->data.begin() + raw->valid_size);
    }

    BenchmarkData Deserialize(const std::string& serialized) override {
        std::vector<uint8_t> raw(serialized.begin(), serialized.end());
        auto& factory = GetFactory();

        auto result = factory.deserializePacket(raw);
        if (!result.has_value()) {
            throw std::runtime_error("PacketForge deserialization failed");
        }

        auto& deser = static_cast<packet_forge::PacketDeserializer<BenchmarkData>&>(
            *result.value().deserializer
        );
        return deser.getArgs();
    }

    size_t GetSerializedSize(const BenchmarkData& data) override {
        auto& factory = GetFactory();
        auto packet = factory.create(
            packet_forge::CommandType<packet_forge::BenchmarkSuit_tag>::BenchmarkCommand,
            data
        );
        return packet.build()->valid_size;
    }

private:
    static packet_forge::CommandFactory<packet_forge::BenchmarkSuit_tag>& GetFactory() {
        static auto factory = [] {
            packet_forge::CommandFactory<packet_forge::BenchmarkSuit_tag> f;
            f.template registerCommand<BenchmarkData>(
                packet_forge::CommandType<packet_forge::BenchmarkSuit_tag>::BenchmarkCommand, {'x'}
            );
            return f;
        }();
        return factory;
    }
};