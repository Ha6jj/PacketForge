#pragma once

#include <PacketForge/serializers/IntSerializers.hpp>

#include <map>

namespace packet_forge {

// Include K and V serializers
template <typename K, typename V>
struct Serializer<std::map<K, V>>
{
    static void serialize(const std::map<K, V>& value, std::vector<uint8_t>& packet)
    {
        uint32_t size = static_cast<uint32_t>(value.size());
        Serializer<uint32_t>::serialize(size, packet);
        for (const auto& [key, val] : value)
        {
            Serializer<K>::serialize(key, packet);
            Serializer<V>::serialize(val, packet);
        }
    }
};

template <typename K, typename V>
struct Deserializer<std::map<K, V>>
{
    static DeserializationResult deserialize(std::map<K, V>& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        uint32_t size;
        if (auto res = Deserializer<uint32_t>::deserialize(size, packet, offset); res != DeserializationResult::Success)
            return res;

        value.clear();
        for (uint32_t i = 0; i < size; ++i)
        {
            K key;
            V val;
            if (auto res = Deserializer<K>::deserialize(key, packet, offset); res != DeserializationResult::Success)
                return res;
            if (auto res = Deserializer<V>::deserialize(val, packet, offset); res != DeserializationResult::Success)
                return res;
            value.emplace(std::move(key), std::move(val));
        }
        return DeserializationResult::Success;
    }
};

} // namespace packet_forge