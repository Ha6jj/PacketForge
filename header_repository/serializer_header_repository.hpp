#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "../command_type.hpp"

class SerializerHeaderRepository
{
public:
    void addHeader(CommandType command, const std::vector<uint8_t>& header);
    bool hasCommand(CommandType command) const noexcept;
    const std::vector<uint8_t>& getHeader(CommandType command) const;

private:
    std::unordered_map<CommandType, std::vector<uint8_t>> headers;
};