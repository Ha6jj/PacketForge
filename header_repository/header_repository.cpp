#include "header_repository.hpp"
#include <stdexcept>

void HeaderRepository::addHeader(CommandType command, const std::vector<uint8_t>& header)
{
    if (header.empty())
    {
        throw std::runtime_error("Empty header is not allowed");
    }

    if (serializer_header_repository.hasCommand(command))
    {
        throw std::runtime_error("Same command already exists");
    }

    serializer_header_repository.addHeader(command, header);
    deserializer_header_repository.addHeader(command, header);
}

std::vector<uint8_t> HeaderRepository::getHeader(CommandType command) const
{
    return serializer_header_repository.getHeader(command);
}

CommandType HeaderRepository::getCommand(const std::vector<uint8_t>& packet) const
{
    return deserializer_header_repository.getCommand(packet);
}