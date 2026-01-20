#pragma once

#include "../detail/CommandType.hpp"
#include "IPacketDeserializer.hpp"

#include <memory>

namespace packet_forge {

template <typename Tag>
struct PacketDescriptor
{
private:
    using SuitType = CommandType<Tag>;

public:
    SuitType command;
    std::unique_ptr<IPacketDeserializer> deserializer;
    size_t totalSize;
};

} // namespace packet_forge
