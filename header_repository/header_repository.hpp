#pragma once

#include <vector>
#include "deserializer_header_repository.hpp"
#include "serializer_header_repository.hpp"

class HeaderRepository
{
public:
    void addHeader(CommandType command, const std::vector<uint8_t>& header);
    std::vector<uint8_t> getHeader(CommandType command) const;
    CommandType getCommand(const std::vector<uint8_t>& packet) const;

private:
    DeserializerHeaderRepository deserializer_header_repository;
    SerializerHeaderRepository serializer_header_repository;
};