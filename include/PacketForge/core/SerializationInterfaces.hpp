#pragma once

#include "DeserializeResult.hpp"
#include "header_repository/VectorView.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace packet_forge {

class ISerializer
{
public:
    virtual ~ISerializer() = default;
    virtual void serialize(std::vector<uint8_t>& packet) const = 0;
};

class IDeserializer
{
public:
    virtual ~IDeserializer() = default;
    virtual DeserializeResult deserialize(VectorView<const uint8_t> packet, size_t& offset) = 0;
};

} // namespace packet_forge
