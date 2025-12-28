#pragma once

#include "CommandType.hpp"

#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace packet_forge {

template <typename Tag>
class SerializerHeaderRepository
{
    using SuitType = CommandType<Tag>;
public:
    void addHeader(SuitType command, const std::vector<uint8_t>& header)
    {
        headers[command] = header;
    }

    bool hasCommand(SuitType command) const noexcept
    {
        return headers.find(command) != headers.end();
    }

    const std::vector<uint8_t>& getHeader(SuitType command) const
    {
        auto it = headers.find(command);
        if (it == headers.end())
        {
            throw std::runtime_error("Command not found in serializer repository");
        }
        return it->second;
    }

private:
    std::unordered_map<SuitType, std::vector<uint8_t>> headers;
};

} // namespace packet_forge
