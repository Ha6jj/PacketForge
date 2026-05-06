#pragma once

#include <PacketForge/impl/serialization/Packet.hpp>
#include <PacketForge/impl/serialization/PacketSerializer.hpp>
#include <PacketForge/impl/deserialization/PacketDescriptor.hpp>
#include <PacketForge/impl/deserialization/PacketDeserializer.hpp>
#include <PacketForge/impl/header_repository/HeaderRepository.hpp>
#include <PacketForge/SharedBufferPool.hpp>
#include <PacketForge/config.hpp>

#include <functional>

namespace packet_forge {

template <typename Tag>
class CommandFactory
{
    using SuitType = CommandType<Tag>;
    static constexpr bool UseBufferPool = CommandSuit<Tag>::use_buffer_pool;

    using BufferPoolType = std::conditional_t<UseBufferPool, 
                                                std::shared_ptr<SharedBufferPool>, 
                                                EmptyBufferPool>;

public:
    CommandFactory() {
        if constexpr (UseBufferPool) {
            buffer_pool_ = std::make_shared<SharedBufferPool>(DEFAULT_BUFFER_POOL_SIZE);
        }
    }

    // Argument will be used only if use_buffer_pool enabled in Tag
    explicit CommandFactory(std::shared_ptr<SharedBufferPool> buffer_pool)
        : buffer_pool_(buffer_pool) {}

    template <typename ArgStruct>
    void registerCommand(SuitType cmd, const std::vector<uint8_t>& header)
    {
        headers.addHeader(cmd, header);
        deserializers_[static_cast<uint32_t>(cmd)] = []
        {
            return std::make_unique<PacketDeserializer<ArgStruct>>();
        };
    }

    template <typename ArgStruct>
    Packet<Tag> create(SuitType cmd, ArgStruct&& args) const
    {
        return Packet<Tag>(
            std::make_unique<PacketSerializer<ArgStruct>>(std::forward<ArgStruct>(args)),
            headers.getHeader(cmd),
            buffer_pool_
        );
    }

    std::optional<PacketDescriptor<Tag>> deserializePacket(VectorView<const uint8_t> packet_view) const
    {
        if (packet_view.empty()) return std::nullopt;

        SuitType command = headers.getCommand(packet_view);
        auto it = deserializers_.find(static_cast<uint32_t>(command));
        if (it == deserializers_.end())
        {
            throw std::invalid_argument("No deserializer registered for command");
        }

        auto deserializer = it->second();
        size_t offset = headers.getHeader(command).size();
        if (deserializer->deserialize(packet_view, offset) != DeserializationResult::Success)
        {
            return std::nullopt;
        }

        return PacketDescriptor<Tag>{ command, std::move(deserializer), offset };
    }

    std::vector<PacketDescriptor<Tag>> deserializeStream(VectorView<const uint8_t> stream_view) const
    {
        std::vector<PacketDescriptor<Tag>> result;

        while (peekCommand(stream_view).has_value())
        {
            auto packetOpt = deserializePacket(stream_view);
            
            if (!packetOpt) break;

            auto& packet = *packetOpt;
            stream_view = stream_view.subspan(packet.totalSize);

            result.push_back(std::move(packet));
        }

        return result;
    }

    std::optional<SuitType> peekCommand(VectorView<const uint8_t> packet) const noexcept
    {
        if (packet.empty()) return std::nullopt;
        return headers.tryGetCommand(packet);
    }


private:
    HeaderRepository<Tag> headers;
    std::unordered_map<uint32_t, std::function<std::unique_ptr<IPacketDeserializer>()>> deserializers_;

    BufferPoolType buffer_pool_;
};

} // namespace packet_forge
