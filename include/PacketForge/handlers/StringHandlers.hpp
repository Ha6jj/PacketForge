#pragma once

#include "IntHandlers.hpp"

#include <string>

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
    static void deserialize(std::string& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        uint32_t length;
        Deserializer<uint32_t>::deserialize(length, packet, offset);
        if (offset + length > packet.size())
        {
            throw std::out_of_range("Packet too short");
        }
        value.assign(packet.begin() + offset, packet.begin() + offset + length);
        offset += length;
    }
};