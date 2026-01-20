#pragma once

#include <cstdint>
#include <vector>

namespace packet_forge {

class IPacketSerializer
{
public:
    virtual ~IPacketSerializer() = default;
    virtual void serialize(std::vector<uint8_t>& packet) const = 0;
};

} // namespace packet_forge