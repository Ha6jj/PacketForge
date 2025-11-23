#pragma once

#include "core/Packet.hpp"
#include "core/CommandSerializer.hpp"

#include <functional>

class CommandFactory
{
public:
    template <typename CommandType, typename ArgStruct>
    void registerCommand(CommandType cmd, const std::vector<uint8_t>& header)
    {
        headers.addHeader(cmd, header);
        deserializers_[static_cast<uint32_t>(cmd)] = []
        {
            return std::make_unique<CommandDeserializer<ArgStruct>>();
        };
    }

    template <typename CommandType, typename ArgStruct>
    Packet create(CommandType cmd, ArgStruct&& args) const
    {
        return Packet(
            cmd,
            std::make_unique<CommandSerializer<ArgStruct>>(std::forward<ArgStruct>(args)),
            headers
        );
    }

    std::pair<CommandType, std::unique_ptr<IDeserializer>>
    deserializePacket(const std::vector<uint8_t>& packet) const
    {
        if (packet.empty())
        {
            throw std::invalid_argument("Empty packet");
        }

        CommandType command = headers.getCommand(packet);
        auto it = deserializers_.find(static_cast<uint32_t>(command));
        if (it == deserializers_.end())
        {
            throw std::invalid_argument("No deserializer registered for command");
        }

        auto deserializer = it->second();
        size_t offset = headers.getHeader(command).size();
        deserializer->deserialize(packet, offset);
        return {command, std::move(deserializer)};
    }


private:
    HeaderRepository headers;
    std::unordered_map<uint32_t, std::function<std::unique_ptr<IDeserializer>()>> deserializers_;
};

#define REGISTER_COMMAND(factory, cmd, arg_struct, header)                  \
    factory.template registerCommand<CommandType, arg_struct>(cmd, header);

