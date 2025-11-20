#pragma once

#include "command_type.hpp"
#include "header_repository/header_repository.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <type_traits>

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

template <typename T>
struct Serializer;

template <typename T>
class CommandSerializer : public ISerializer
{
public:
    CommandSerializer(T&& args) : args_(std::move(args)) {}

    void serialize(std::vector<uint8_t>& packet) const override
    {
        Serializer<T>::serialize(args_, packet);
    }

private:
    T args_;
};

template <typename T>
struct Deserializer;

template <typename T>
class CommandDeserializer : public IDeserializer
{
public:
    CommandDeserializer() = default;

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

class Packet
{
public:
    template <typename Command>
    Packet(Command cmd, std::unique_ptr<ISerializer> serializer, const HeaderRepository& header_repo)
        : header_(header_repo.getHeader(cmd)), serializer_(std::move(serializer)) {}

    std::vector<uint8_t> build() const
    {
        std::vector<uint8_t> packet;
        packet.insert(packet.end(), header_.begin(), header_.end());
        if (serializer_)
        {
            serializer_->serialize(packet);
        }
        return packet;
    }

private:
    std::vector<uint8_t> header_;
    std::unique_ptr<ISerializer> serializer_;
};

class CommandFactory
{
public:
    CommandFactory() = default;

    template <typename Command, typename ArgStruct>
    void registerCommand(Command cmd, const std::vector<uint8_t>& header)
    {
        headers.addHeader(cmd, header);
        
        deserializers_[static_cast<uint32_t>(cmd)] = []()
        {
            return std::make_unique<CommandDeserializer<ArgStruct>>();
        };
    }

    template <typename Command, typename ArgStruct>
    Packet create(Command cmd, ArgStruct&& args) const
    {
        return Packet(
            cmd,
            std::make_unique<CommandSerializer<ArgStruct>>(std::forward<ArgStruct>(args)),
            headers);
    }

    std::pair<CommandType, std::unique_ptr<IDeserializer>> deserializePacket(const std::vector<uint8_t>& packet) const
    {
        if (packet.empty())
        {
            throw std::invalid_argument("Empty packet");
        }

        CommandType command = headers.getCommand(packet);

        auto it = deserializers_.find(static_cast<uint32_t>(command));
        if (it == deserializers_.end())
        {
            throw std::invalid_argument("No deserializer registered for command");
        }

        auto deserializer = it->second();
        size_t offset = headers.getHeader(command).size();
        deserializer->deserialize(packet, offset);
        return {command, std::move(deserializer)};
    }

private:
    HeaderRepository headers;
    std::unordered_map<uint32_t, std::function<std::unique_ptr<IDeserializer>()> > deserializers_;
};

template <typename T>
using base_type = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename struct_name, typename... Members>
void serialize_members(const struct_name& value, std::vector<uint8_t>& packet, Members... members)
{
    (Serializer<base_type<decltype(value.*members)>>::serialize(value.*members, packet), ...);
}

template <typename struct_name, typename... Members>
void deserialize_members(struct_name& value, const std::vector<uint8_t>& packet, size_t& offset, Members... members)
{
    (Deserializer<base_type<decltype(value.*members)>>::deserialize(value.*members, packet, offset), ...);
}

#define REGISTER_COMMAND(factory, cmd, arg_struct, header)                          \
    factory.registerCommand<CommandType, arg_struct>(cmd, header);

#define PACKET_STRUCTURE(struct_name, ...)                                          \
template <>                                                                         \
struct Serializer<struct_name>                                                      \
{                                                                                   \
    static void serialize(const struct_name& value, std::vector<uint8_t>& packet)   \
    {                                                                               \
        serialize_members(value, packet, __VA_ARGS__);                              \
    }                                                                               \
};                                                                                  \
                                                                                    \
template <>                                                                         \
struct Deserializer<struct_name>                                                    \
{                                                                                   \
    static void deserialize(struct_name& value,                                     \
                            const std::vector<uint8_t>& packet,                     \
                            size_t& offset)                                         \
    {                                                                               \
        deserialize_members(value, packet, offset, __VA_ARGS__);                    \
    }                                                                               \
};                                                                                  \

// Serializer spechialisations
template <>
struct Serializer<uint8_t>
{
    static void serialize(const uint8_t& value, std::vector<uint8_t>& packet)
    {
        packet.push_back(value);
    }
};

template <>
struct Serializer<uint16_t>
{
    static void serialize(const uint16_t& value, std::vector<uint8_t>& packet)
    {
        packet.push_back(static_cast<uint8_t>(value & 0xff));
        packet.push_back(static_cast<uint8_t>((value >> 8) & 0xff));
    }
};

template <>
struct Serializer<uint32_t>
{
    static void serialize(const uint32_t& value, std::vector<uint8_t>& packet)
    {
        uint32_t val = value;
        for (int i = 0; i < 4; ++i)
        {
            packet.push_back(static_cast<uint8_t>(val & 0xff));
            val >>= 8;
        }
    }
};

template <>
struct Serializer<std::string>
{
    static void serialize(const std::string& value, std::vector<uint8_t>& packet)
    {
        uint32_t length = static_cast<uint32_t>(value.size());
        Serializer<uint32_t>::serialize(length, packet);
        packet.insert(packet.end(), value.begin(), value.end());
    }
};

// Deserializer spechialisations
template <>
struct Deserializer<uint8_t>
{
    static void deserialize(uint8_t& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset >= packet.size())
        {
            throw std::out_of_range("Packet too short");
        }
        value = packet[offset++];
    }
};

template <>
struct Deserializer<uint16_t>
{
    static void deserialize(uint16_t& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset + 2 > packet.size())
        {
            throw std::out_of_range("Packet too short");
        }
        value = static_cast<uint16_t>(packet[offset]) | (static_cast<uint16_t>(packet[offset + 1]) << 8);
        offset += 2;
    }
};

template <>
struct Deserializer<uint32_t>
{
    static void deserialize(uint32_t& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset + 4 > packet.size())
        {
            throw std::out_of_range("Packet too short");
        }
        value = 0;
        for (int i = 0; i < 4; ++i)
        {
            value |= static_cast<uint32_t>(packet[offset++]) << (i * 8);
        }
    }
};

template <>
struct Deserializer<std::string>
{
    static void deserialize(std::string& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        uint32_t length;
        Deserializer<uint32_t>::deserialize(length, packet, offset);
        if (offset + length > packet.size())
        {
            throw std::out_of_range("Packet too short");
        }
        value.assign(packet.begin() + offset, packet.begin() + offset + length);
        offset += length;
    }
};
