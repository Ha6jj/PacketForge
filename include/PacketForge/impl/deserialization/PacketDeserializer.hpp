#pragma once

#include <PacketForge/impl/deserialization/Deserializer.hpp>
#include <PacketForge/impl/deserialization/IPacketDeserializer.hpp>
#include <PacketForge/impl/deserialization/DeserializationResult.hpp>
#include <PacketForge/impl/detail/vector_view/VectorView.hpp>

namespace packet_forge {

template <typename T>
class PacketDeserializer : public IPacketDeserializer
{
public:
    DeserializationResult deserialize(VectorView<const uint8_t> packet_view, size_t& offset) override
    {
        return Deserializer<T>::deserialize(args_, packet_view, offset);
    }

    T& getArgs()
    {
        return args_;
    }

private:
    T args_;
};

} // namespace packet_forge
