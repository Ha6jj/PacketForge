#pragma once

#include "../SerializerTraits.hpp"

#include <stdexcept>

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
    static void deserialize(uint8_t& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset >= packet.size()) throw std::out_of_range("Packet too short");

        value = packet[offset++];
    }
};

template <>
struct Deserializer<uint16_t>
{
    static void deserialize(uint16_t& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset + 2 > packet.size()) throw std::out_of_range("Packet too short");

        value = static_cast<uint16_t>(packet[offset]) 
                | (static_cast<uint16_t>(packet[offset + 1]) << 8);
            offset += 2;
    }
};

template <>
struct Deserializer<uint32_t>
{
    static void deserialize(uint32_t& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset + 4 > packet.size()) throw std::out_of_range("Packet too short");

        value = 0;
        for (int i = 0; i < 4; ++i)
        {
            value |= static_cast<uint32_t>(packet[offset++]) << (i * 8);
        }
    }
};

} // namespace packet_forge
