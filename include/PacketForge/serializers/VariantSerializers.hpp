#pragma once
#include <PacketForge/serializers/IntSerializers.hpp>
#include <variant>

namespace packet_forge {

template <typename... Ts>
struct Serializer<std::variant<Ts...>>
{
    static void serialize(const std::variant<Ts...>& value, std::vector<uint8_t>& packet)
    {
        uint32_t index = static_cast<uint32_t>(value.index());
        Serializer<uint32_t>::serialize(index, packet);

        std::visit([&packet](const auto& v) {
            using U = std::decay_t<decltype(v)>;
            Serializer<U>::serialize(v, packet);
        }, value);
    }
};

template <typename... Ts>
struct Deserializer<std::variant<Ts...>>
{
    static DeserializationResult deserialize(std::variant<Ts...>& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        uint32_t index;
        if (auto res = Deserializer<uint32_t>::deserialize(index, packet, offset); res != DeserializationResult::Success)
            return res;

        if (index >= sizeof...(Ts))
            return DeserializationResult::OutOfRange;

        return deserialize_impl<0, Ts...>(index, value, packet, offset);
    }

private:
    template <size_t I, typename T, typename... Rest>
    static DeserializationResult deserialize_impl(uint32_t index, std::variant<Ts...>& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (index == I)
        {
            T item;
            if (auto res = Deserializer<T>::deserialize(item, packet, offset); res != DeserializationResult::Success)
                return res;
            
            value = std::move(item);
            return DeserializationResult::Success;
        }

        if constexpr (sizeof...(Rest) > 0)
        {
            return deserialize_impl<I + 1, Rest...>(index, value, packet, offset);
        }
        
        return DeserializationResult::OutOfRange;
    }
};

} // namespace packet_forge
