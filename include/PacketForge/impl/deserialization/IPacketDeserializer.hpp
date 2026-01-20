#pragma once

#include "DeserializationResult.hpp"
#include "../detail/vector_view/VectorView.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace packet_forge {

class IPacketDeserializer
{
public:
    virtual ~IPacketDeserializer() = default;
    virtual DeserializationResult deserialize(VectorView<const uint8_t> packet, size_t& offset) = 0;
};

} // namespace packet_forge
