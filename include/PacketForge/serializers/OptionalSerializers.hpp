#pragma once
#include <PacketForge/serializers/IntSerializers.hpp>
#include <optional>

namespace packet_forge {

template <typename T>
struct Serializer<std::optional<T>>
{
    static void serialize(const std::optional<T>& value, std::vector<uint8_t>& packet)
    {
        bool has_value = value.has_value();
        Serializer<bool>::serialize(has_value, packet);
        if (has_value)
        {
            Serializer<T>::serialize(*value, packet);
        }
    }
};

template <typename T>
struct Deserializer<std::optional<T>>
{
    static DeserializationResult deserialize(std::optional<T>& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        bool has_value;
        if (auto res = Deserializer<bool>::deserialize(has_value, packet, offset); res != DeserializationResult::Success)
            return res;

        if (has_value)
        {
            T inner;
            if (auto res = Deserializer<T>::deserialize(inner, packet, offset); res != DeserializationResult::Success)
                return res;
            value = std::move(inner);
        }
        else
        {
            value.reset();
        }
        return DeserializationResult::Success;
    }
};

} // namespace packet_forge
