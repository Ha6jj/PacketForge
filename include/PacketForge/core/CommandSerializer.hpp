#pragma once

#include "Serializer.hpp"
#include "SerializationInterfaces.hpp"

#include <memory>
#include <utility>

template <typename T>
class CommandSerializer : public ISerializer
{
public:
    explicit CommandSerializer(T&& args)
        : args_(std::forward<T>(args)) {}

    void serialize(std::vector<uint8_t>& packet) const override
    {
        Serializer<T>::serialize(args_, packet);
    }

private:
    T args_;
};

template <typename T>
class CommandDeserializer : public IDeserializer
{
public:
    void deserialize(const std::vector<uint8_t>& packet, size_t& offset) override
    {
        Deserializer<T>::deserialize(args_, packet, offset);
    }

    T& getArgs()
    {
        return args_;
    }

private:
    T args_;
};