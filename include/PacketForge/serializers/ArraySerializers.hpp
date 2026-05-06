#pragma once

#include <PacketForge/serializers/IntSerializers.hpp>

#include <array>
#include <vector>
#include <cstring>

namespace packet_forge {

template <typename T, size_t N>
struct Serializer<std::array<T, N>>
{
    static void serialize(const std::array<T, N>& value, std::vector<uint8_t>& packet)
    {
#if PACKET_FORGE_ENABLE_CONTAINER_FAST_SERIALIZATION
        if constexpr (std::is_trivially_copyable_v<T>) {
            if constexpr (N == 0) return;
            
            size_t old_size = packet.size();
            size_t total_bytes = N * sizeof(T);
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

template <typename T, size_t N>
struct Deserializer<std::array<T, N>>
{
    static DeserializationResult deserialize(std::array<T, N>& value, VectorView<const uint8_t> packet, size_t& offset)
    {
#if PACKET_FORGE_ENABLE_CONTAINER_FAST_SERIALIZATION
        if constexpr (std::is_trivially_copyable_v<T>) {
            size_t total_bytes = N * sizeof(T);
            if (offset + total_bytes > packet.size()) 
                return DeserializationResult::OutOfRange;
            
            std::memcpy(value.data(), packet.data() + offset, total_bytes);
            offset += total_bytes;
            return DeserializationResult::Success;
        }
#endif
        for (size_t i = 0; i < N; ++i)
        {
            if (auto res = Deserializer<T>::deserialize(value[i], packet, offset); res != DeserializationResult::Success)
                return res;
        }
        return DeserializationResult::Success;
    }
};

} // namespace packet_forge
