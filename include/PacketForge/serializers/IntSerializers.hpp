#pragma once

#include "../SerializerKit.hpp"

namespace packet_forge {

// Serializer specializations
template <>
struct Serializer<uint8_t>
{
    static void serialize(uint8_t value, std::vector<uint8_t>& packet)
    {
        packet.push_back(value);
    }
};

template <>
struct Serializer<uint16_t>
{
    static void serialize(uint16_t value, std::vector<uint8_t>& packet)
    {
        packet.push_back(static_cast<uint8_t>(value & 0xFF));
        packet.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    }
};

template <>
struct Serializer<uint32_t>
{
    static void serialize(uint32_t value, std::vector<uint8_t>& packet)
    {
        for (int i = 0; i < 4; ++i)
        {
            packet.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }
};

// Deserializer specializations
template <>
struct Deserializer<uint8_t>
{
    static DeserializationResult deserialize(uint8_t& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 1 > packet.size()) return DeserializationResult::OutOfRange;

        value = packet[offset++];
        return DeserializationResult::Success;
    }
};

template <>
struct Deserializer<uint16_t>
{
    static DeserializationResult deserialize(uint16_t& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 2 > packet.size()) return DeserializationResult::OutOfRange;

        value = static_cast<uint16_t>(packet[offset]) 
                | (static_cast<uint16_t>(packet[offset + 1]) << 8);
        offset += 2;
        return DeserializationResult::Success;
    }
};

template <>
struct Deserializer<uint32_t>
{
    static DeserializationResult deserialize(uint32_t& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 4 > packet.size()) return DeserializationResult::OutOfRange;

        value = 0;
        for (int i = 0; i < 4; ++i)
        {
            value |= static_cast<uint32_t>(packet[offset++]) << (i * 8);
        }
        return DeserializationResult::Success;
    }
};

} // namespace packet_forge
