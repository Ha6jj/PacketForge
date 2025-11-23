#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class ISerializer
{
public:
    virtual ~ISerializer() = default;
    virtual void serialize(std::vector<uint8_t>& packet) const = 0;
};

class IDeserializer
{
public:
    virtual ~IDeserializer() = default;
    virtual void deserialize(const std::vector<uint8_t>& packet, size_t& offset) = 0;
};