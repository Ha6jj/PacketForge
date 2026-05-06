#pragma once

#include <PacketForge/SerializerKit.hpp>
#include <cstring>

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

template <>
struct Serializer<uint64_t>
{
    static void serialize(uint64_t value, std::vector<uint8_t>& packet)
    {
        for (int i = 0; i < 8; ++i)
        {
            packet.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }
};

template <>
struct Serializer<bool>
{
    static void serialize(bool value, std::vector<uint8_t>& packet)
    {
        Serializer<uint8_t>::serialize(value ? 1 : 0, packet);
    }
};

template <>
struct Serializer<float>
{
    static void serialize(float value, std::vector<uint8_t>& packet)
    {
        static_assert(sizeof(float) == sizeof(uint32_t), "float must be 32-bit IEEE 754");
        uint32_t raw;
        std::memcpy(&raw, &value, sizeof(float));
        for (int i = 0; i < 4; ++i)
        {
            packet.push_back(static_cast<uint8_t>(raw & 0xFF));
            raw >>= 8;
        }
    }
};

template <>
struct Serializer<double>
{
    static void serialize(double value, std::vector<uint8_t>& packet)
    {
        static_assert(sizeof(double) == sizeof(uint64_t), "double must be 64-bit IEEE 754");
        uint64_t raw;
        std::memcpy(&raw, &value, sizeof(double));
        for (int i = 0; i < 8; ++i)
        {
            packet.push_back(static_cast<uint8_t>(raw & 0xFF));
            raw >>= 8;
        }
    }
};

// Serializer specializations for signed types
template <>
struct Serializer<int8_t>
{
    static void serialize(int8_t value, std::vector<uint8_t>& packet)
    {
        packet.push_back(static_cast<uint8_t>(value));
    }
};

template <>
struct Serializer<int16_t>
{
    static void serialize(int16_t value, std::vector<uint8_t>& packet)
    {
        // Приведение к uint16_t сохраняет битовый паттерн в двух дополнении
        uint16_t raw = static_cast<uint16_t>(value);
        packet.push_back(static_cast<uint8_t>(raw & 0xFF));
        packet.push_back(static_cast<uint8_t>((raw >> 8) & 0xFF));
    }
};

template <>
struct Serializer<int32_t>
{
    static void serialize(int32_t value, std::vector<uint8_t>& packet)
    {
        uint32_t raw = static_cast<uint32_t>(value);
        for (int i = 0; i < 4; ++i)
        {
            packet.push_back(static_cast<uint8_t>(raw & 0xFF));
            raw >>= 8;
        }
    }
};

template <>
struct Serializer<int64_t>
{
    static void serialize(int64_t value, std::vector<uint8_t>& packet)
    {
        uint64_t raw = static_cast<uint64_t>(value);
        for (int i = 0; i < 8; ++i)
        {
            packet.push_back(static_cast<uint8_t>(raw & 0xFF));
            raw >>= 8;
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

template <>
struct Deserializer<uint64_t>
{
    static DeserializationResult deserialize(uint64_t& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 8 > packet.size()) return DeserializationResult::OutOfRange;

        value = 0;
        for (int i = 0; i < 8; ++i)
        {
            value |= static_cast<uint64_t>(packet[offset++]) << (i * 8);
        }
        return DeserializationResult::Success;
    }
};

template <>
struct Deserializer<bool>
{
    static DeserializationResult deserialize(bool& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        uint8_t raw;
        if (auto res = Deserializer<uint8_t>::deserialize(raw, packet, offset); res != DeserializationResult::Success)
            return res;
        value = (raw != 0);
        return DeserializationResult::Success;
    }
};

template <>
struct Deserializer<float> {
    static DeserializationResult deserialize(float& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 4 > packet.size()) return DeserializationResult::OutOfRange;
        uint32_t raw = 0;
        for (int i = 0; i < 4; ++i)
        {
            raw |= static_cast<uint32_t>(packet[offset++]) << (i * 8);
        }

        std::memcpy(&value, &raw, sizeof(float));
        return DeserializationResult::Success;
    }
};

template <>
struct Deserializer<double> {
    static DeserializationResult deserialize(double& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 8 > packet.size()) return DeserializationResult::OutOfRange;
        uint64_t raw = 0;
        for (int i = 0; i < 8; ++i)
        {
            raw |= static_cast<uint64_t>(packet[offset++]) << (i * 8);
        }

        std::memcpy(&value, &raw, sizeof(double));
        return DeserializationResult::Success;
    }
};

// Deserializer specializations for signed types
template <>
struct Deserializer<int8_t>
{
    static DeserializationResult deserialize(int8_t& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 1 > packet.size()) return DeserializationResult::OutOfRange;

        value = static_cast<int8_t>(packet[offset++]);
        return DeserializationResult::Success;
    }
};

template <>
struct Deserializer<int16_t>
{
    static DeserializationResult deserialize(int16_t& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 2 > packet.size()) return DeserializationResult::OutOfRange;

        uint16_t raw = static_cast<uint16_t>(packet[offset]) 
                     | (static_cast<uint16_t>(packet[offset + 1]) << 8);
        value = static_cast<int16_t>(raw);
        offset += 2;
        return DeserializationResult::Success;
    }
};

template <>
struct Deserializer<int32_t>
{
    static DeserializationResult deserialize(int32_t& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 4 > packet.size()) return DeserializationResult::OutOfRange;

        uint32_t raw = 0;
        for (int i = 0; i < 4; ++i)
        {
            raw |= static_cast<uint32_t>(packet[offset++]) << (i * 8);
        }
        value = static_cast<int32_t>(raw);
        return DeserializationResult::Success;
    }
};

template <>
struct Deserializer<int64_t>
{
    static DeserializationResult deserialize(int64_t& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (offset + 8 > packet.size()) return DeserializationResult::OutOfRange;

        uint64_t raw = 0;
        for (int i = 0; i < 8; ++i)
        {
            raw |= static_cast<uint64_t>(packet[offset++]) << (i * 8);
        }
        value = static_cast<int64_t>(raw);
        return DeserializationResult::Success;
    }
};

} // namespace packet_forge
