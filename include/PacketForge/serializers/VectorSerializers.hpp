#pragma once

#include "../SerializerKit.hpp"
#include "IntSerializers.hpp"

#include <vector>

namespace packet_forge {

// Include T serializer
template <typename T>
struct Serializer<std::vector<T>>
{
    static void serialize(const std::vector<T>& value, std::vector<uint8_t>& packet)
    {
        uint32_t size = static_cast<uint32_t>(value.size());
        Serializer<uint32_t>::serialize(size, packet);

#if PACKET_FORGE_ENABLE_CONTAINER_FAST_SERIALIZATION
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (size == 0) return;
            size_t old_size = packet.size();
            size_t total_bytes = size * sizeof(T);
            packet.resize(old_size + total_bytes);
            std::memcpy(packet.data() + old_size, value.data(), total_bytes);
            return;
        }
#endif

        for (const auto& item : value)
        {
            Serializer<T>::serialize(item, packet);
        }
    }
};

template <typename T>
struct Deserializer<std::vector<T>>
{
    static DeserializationResult deserialize(std::vector<T>& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        uint32_t size;
        if (auto res = Deserializer<uint32_t>::deserialize(size, packet, offset); res != DeserializationResult::Success)
            return res;

#if PACKET_FORGE_ENABLE_CONTAINER_FAST_SERIALIZATION
        if constexpr (std::is_trivially_copyable_v<T>) {
            size_t total_bytes = size * sizeof(T);
            if (offset + total_bytes > packet.size()) return DeserializationResult::OutOfRange;
            value.resize(size);
            std::memcpy(value.data(), packet.data() + offset, total_bytes);
            offset += total_bytes;
            return DeserializationResult::Success;
        }
#endif

        value.clear();
        value.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            T item;
            if (auto res = Deserializer<T>::deserialize(item, packet, offset); res != DeserializationResult::Success)
                return res;
            value.push_back(std::move(item));
        }
        return DeserializationResult::Success;
    }
};

} // namespace packet_forge