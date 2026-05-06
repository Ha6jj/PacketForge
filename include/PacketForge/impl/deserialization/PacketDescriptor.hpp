#pragma once

#include <PacketForge/impl/detail/CommandType.hpp>
#include <PacketForge/impl/deserialization/IPacketDeserializer.hpp>

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
