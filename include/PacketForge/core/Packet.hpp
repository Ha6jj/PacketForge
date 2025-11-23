#pragma once

#include "SerializationInterfaces.hpp"
#include "header_repository/HeaderRepository.hpp"

#include <vector>
#include <memory>

class Packet
{
public:
    template <typename Command>
    Packet(Command cmd, 
           std::unique_ptr<ISerializer> serializer,
           const HeaderRepository& header_repo)
        : header_(header_repo.getHeader(cmd)), serializer_(std::move(serializer)) {}

    std::vector<uint8_t> build() const
    {
        std::vector<uint8_t> packet;
        packet.insert(packet.end(), header_.begin(), header_.end());
        if (serializer_) serializer_->serialize(packet);
        return packet;
    }

private:
    std::vector<uint8_t> header_;
    std::unique_ptr<ISerializer> serializer_;
};