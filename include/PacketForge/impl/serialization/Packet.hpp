#pragma once

#include <PacketForge/impl/serialization/IPacketSerializer.hpp>
#include <PacketForge/impl/detail/CommandType.hpp>
#include <PacketForge/impl/detail/vector_view/VectorView.hpp>
#include <PacketForge/SharedBufferPool.hpp>

#include <memory>

namespace packet_forge {

// Plug if buffer_pool not enabled in Tag
struct EmptyBufferPool {
    constexpr EmptyBufferPool() = default;
    explicit EmptyBufferPool(std::shared_ptr<SharedBufferPool>) noexcept {}
};

template <typename Tag>
class Packet
{
    using SuitType = CommandType<Tag>;
    static constexpr bool UseBufferPool = CommandSuit<Tag>::use_buffer_pool;

    using BufferPoolType = std::conditional_t<UseBufferPool, 
                                                std::shared_ptr<SharedBufferPool>, 
                                                EmptyBufferPool>;
public:
    Packet(std::unique_ptr<IPacketSerializer> serializer,
           VectorView<const uint8_t> header, 
           BufferPoolType buffer_pool = {})
        : header_(header)
        , serializer_(std::move(serializer))
        , buffer_pool_(buffer_pool) {}

    auto build() const
    {
        if constexpr (UseBufferPool)
        {
            SharedBuffer buf = buffer_pool_->acquire();
            
            buf->data.clear();
            buf->data.insert(buf->data.end(), header_.begin(), header_.end());
            
            if (serializer_) serializer_->serialize(buf->data);
            
            buf->valid_size = buf->data.size();
            return buf;
        }
        else
        {
            std::vector<uint8_t> packet;
            packet.reserve(header_.size() + 1024);
            packet.insert(packet.end(), header_.begin(), header_.end());
            if (serializer_) serializer_->serialize(packet);
            return packet;
        }
    }

private:
    VectorView<const uint8_t> header_;
    std::unique_ptr<IPacketSerializer> serializer_;
    BufferPoolType buffer_pool_;
};

} // namespace packet_forge
