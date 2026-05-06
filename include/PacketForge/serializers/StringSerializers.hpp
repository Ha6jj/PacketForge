#pragma once

#include <PacketForge/serializers/IntSerializers.hpp>

#include <string>

namespace packet_forge {

template <>
struct Serializer<std::string>
{
    static void serialize(const std::string& value, std::vector<uint8_t>& packet)
    {
        uint32_t length = static_cast<uint32_t>(value.size());
        Serializer<uint32_t>::serialize(length, packet);
        packet.insert(packet.end(), value.begin(), value.end());
    }
};

template <>
struct Deserializer<std::string>
{
    static DeserializationResult deserialize(std::string& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        uint32_t length;
        if (auto res = Deserializer<uint32_t>::deserialize(length, packet, offset);
            res != DeserializationResult::Success) return res;
        if (offset + length > packet.size()) return DeserializationResult::OutOfRange;

        value.assign(packet.begin() + offset, packet.begin() + offset + length);
        offset += length;
        return DeserializationResult::Success;
    }
};

} // namespace packet_forge
