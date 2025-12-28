#pragma once

#include "CommandType.hpp"

#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace packet_forge {

class SerializerHeaderRepository
{
public:
    void addHeader(CommandType command, const std::vector<uint8_t>& header)
    {
        headers[command] = header;
    }

    bool hasCommand(CommandType command) const noexcept
    {
        return headers.find(command) != headers.end();
    }

    const std::vector<uint8_t>& getHeader(CommandType command) const
    {
        auto it = headers.find(command);
        if (it == headers.end())
        {
            throw std::runtime_error("Command not found in serializer repository");
        }
        return it->second;
    }

private:
    std::unordered_map<CommandType, std::vector<uint8_t>> headers;
};

} // namespace packet_forge
