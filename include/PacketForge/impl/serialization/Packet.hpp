#pragma once

#include "IPacketSerializer.hpp"
#include "../detail/CommandType.hpp"
#include "../detail/vector_view/VectorView.hpp"

#include <memory>

namespace packet_forge {

template <typename Tag>
class Packet
{
    using SuitType = CommandType<Tag>;
public:
    Packet(SuitType cmd, 
           std::unique_ptr<IPacketSerializer> serializer,
           VectorView<const uint8_t> header)
        : header_(header), serializer_(std::move(serializer)) {}

    std::vector<uint8_t> build() const
    {
        std::vector<uint8_t> packet;
        packet.insert(packet.end(), header_.begin(), header_.end());
        if (serializer_) serializer_->serialize(packet);
        return packet;
    }

private:
    VectorView<const uint8_t> header_;
    std::unique_ptr<IPacketSerializer> serializer_;
};

} // namespace packet_forge
