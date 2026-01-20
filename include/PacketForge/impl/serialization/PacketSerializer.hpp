#pragma once

#include "Serializer.hpp"
#include "IPacketSerializer.hpp"

#include <memory>

namespace packet_forge {

template <typename T>
class PacketSerializer : public IPacketSerializer
{
public:
    explicit PacketSerializer(T&& args)
        : args_(std::forward<T>(args)) {}

    void serialize(std::vector<uint8_t>& packet) const override
    {
        Serializer<T>::serialize(args_, packet);
    }

private:
    T args_;
};

} // namespace packet_forge
