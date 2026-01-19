#pragma once

#include "core/Packet.hpp"
#include "core/CommandSerializer.hpp"

#include <functional>

namespace packet_forge {

template <typename Tag>
class CommandFactory
{
    using SuitType = CommandType<Tag>;
public:
    template <typename ArgStruct>
    void registerCommand(SuitType cmd, const std::vector<uint8_t>& header)
    {
        headers.addHeader(cmd, header);
        deserializers_[static_cast<uint32_t>(cmd)] = []
        {
            return std::make_unique<CommandDeserializer<ArgStruct>>();
        };
    }

    template <typename ArgStruct>
    Packet<Tag> create(SuitType cmd, ArgStruct&& args) const
    {
        return Packet<Tag>(
            cmd,
            std::make_unique<CommandSerializer<ArgStruct>>(std::forward<ArgStruct>(args)),
            headers
        );
    }

    struct PacketResult
    {
        SuitType command;
        std::unique_ptr<IDeserializer> deserializer;
        size_t totalSize;
    };

    std::optional<PacketResult> deserializePacket(VectorView<const uint8_t> packet_view) const
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
        if (deserializer->deserialize(packet_view, offset) != DeserializeResult::DeserializeSuccess)
        {
            return std::nullopt;
        }

        return PacketResult{ command, std::move(deserializer), offset };
    }

    std::vector<PacketResult> deserializeStream(VectorView<const uint8_t> stream_view) const
    {
        std::vector<PacketResult> result;

        while (!stream_view.empty())
        {
            auto packetOpt = deserializePacket(stream_view);
            
            if (!packetOpt) break;

            auto& packet = *packetOpt;
            stream_view = stream_view.subspan(packet.totalSize);

            result.push_back(std::move(packet));
        }

        return result;
    }


private:
    HeaderRepository<Tag> headers;
    std::unordered_map<uint32_t, std::function<std::unique_ptr<IDeserializer>()>> deserializers_;
};

} // namespace packet_forge

#define REGISTER_COMMAND(factory, cmd, arg_struct, header)                  \
    factory.template registerCommand<arg_struct>(cmd, header);
