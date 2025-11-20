#include "serializer_header_repository.hpp"
#include <stdexcept>

void SerializerHeaderRepository::addHeader(CommandType command, const std::vector<uint8_t>& header)
{
    headers[command] = header;
}

bool SerializerHeaderRepository::hasCommand(CommandType command) const noexcept
{
    return headers.find(command) != headers.end();
}

const std::vector<uint8_t>& SerializerHeaderRepository::getHeader(CommandType command) const
{
    auto it = headers.find(command);
    if (it == headers.end())
    {
        throw std::runtime_error("Command not found in serializer repository");
    }
    return it->second;
}