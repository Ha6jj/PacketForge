#pragma once

#include "DeserializerHeaderRepository.hpp"
#include "SerializerHeaderRepository.hpp"

#include <vector>

namespace packet_forge {

template <typename Tag>
class HeaderRepository
{
    using SuitType = CommandType<Tag>;
public:
    void addHeader(SuitType command, const std::vector<uint8_t>& header)
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

    std::vector<uint8_t> getHeader(SuitType command) const
    {
        return serializer_header_repository.getHeader(command);
    }

    SuitType getCommand(const std::vector<uint8_t>& packet) const
    {
        return deserializer_header_repository.getCommand(packet);
    }

private:
    DeserializerHeaderRepository<Tag> deserializer_header_repository;
    SerializerHeaderRepository<Tag> serializer_header_repository;
};

} // namespace packet_forge
