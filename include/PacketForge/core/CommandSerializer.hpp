#pragma once

#include "Serializer.hpp"
#include "SerializationInterfaces.hpp"

#include <memory>
#include <utility>

namespace packet_forge {

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
    DeserializeResult deserialize(VectorView<const uint8_t> packet_view, size_t& offset) override
    {
        return Deserializer<T>::deserialize(args_, packet_view, offset);
    }

    T& getArgs()
    {
        return args_;
    }

private:
    T args_;
};

} // namespace packet_forge
