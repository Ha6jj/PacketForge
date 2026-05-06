#pragma once

#include "schemas.hpp"

#include "PacketForge/CommandFactory.hpp"

#include <vector>
#include <string>
#include <stdexcept>

template <typename Data, 
          packet_forge::CommandType<packet_forge::BenchmarkSuit_tag> Command, 
          const char* Name>
class DataSerializer : public ISerializer<Data> {
public:
    std::string GetName() const override { return Name; }

    std::string Serialize(const Data& data) override {
        auto& factory = GetFactory();
        auto packet = factory.create(Command, data);

        packet_forge::SharedBuffer raw = packet.build();
        return std::string(raw->data.begin(), raw->data.begin() + raw->valid_size);
    }

    Data Deserialize(const std::string& serialized) override {
        auto& factory = GetFactory();

        packet_forge::VectorView<const uint8_t> vv(serialized);
        auto result = factory.deserializePacket(vv);
        if (!result.has_value()) {
            throw std::runtime_error("PacketForge deserialization failed");
        }

        auto& deser = static_cast<packet_forge::PacketDeserializer<Data>&>(
            *result.value().deserializer
        );
        return deser.getArgs();
    }

    size_t GetSerializedSize(const Data& data) override {
        auto& factory = GetFactory();
        auto packet = factory.create(Command, data);
        return packet.build()->valid_size;
    }

private:
    static packet_forge::CommandFactory<packet_forge::BenchmarkSuit_tag>& GetFactory() {
        static auto factory = [] {
            packet_forge::CommandFactory<packet_forge::BenchmarkSuit_tag> f;
            f.registerCommand<Data>(Command, {'x'});
            return f;
        }();
        return factory;
    }
};