#pragma once

#include "UserCommandType.hpp"
#include "PacketForge/macros/PacketStructure.hpp"
#include "PacketForge/serializers/IntSerializers.hpp"
#include "PacketForge/serializers/StringSerializers.hpp"

struct Position
{
    uint32_t x;
    uint32_t y;
};

template <>
struct packet_forge::Serializer<Position>
{
    static void serialize(const Position& value, std::vector<uint8_t>& packet)
    {
        Serializer<uint32_t>::serialize(value.x, packet);
        Serializer<uint32_t>::serialize(value.y, packet);
    }
};

template <>
struct packet_forge::Deserializer<Position>
{
    static DeserializationResult deserialize(Position& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (auto res = Deserializer<uint32_t>::deserialize(value.x, packet, offset);
            res != DeserializationResult::Success) return res;
        if (auto res = Deserializer<uint32_t>::deserialize(value.y, packet, offset);
            res != DeserializationResult::Success) return res;
        return DeserializationResult::Success;
        // return Deserializer<uint32_t>::deserialize(value.x, packet, offset);
    }
};

struct SomeNote
{
    uint8_t system_flags;
    std::string note;
};

template <>
struct packet_forge::Serializer<SomeNote>
{
    static void serialize(const SomeNote& value, std::vector<uint8_t>& packet)
    {
        Serializer<uint8_t>::serialize(value.system_flags, packet);
        Serializer<std::string>::serialize(value.note, packet);
    }
};

template <>
struct packet_forge::Deserializer<SomeNote>
{
    static DeserializationResult deserialize(SomeNote& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (auto res = Deserializer<uint8_t>::deserialize(value.system_flags, packet, offset);
            res != DeserializationResult::Success) return res;
        if (auto res = Deserializer<std::string>::deserialize(value.note, packet, offset);
            res != DeserializationResult::Success) return res;
        return DeserializationResult::Success;
    }
};

struct Entity
{
    Position pos;
    std::string name;
};

template <>
struct packet_forge::Serializer<Entity>
{
    static void serialize(const Entity& value, std::vector<uint8_t>& packet)
    {
        Serializer<Position>::serialize(value.pos, packet);
        Serializer<std::string>::serialize(value.name, packet);
    }
};

template <>
struct packet_forge::Deserializer<Entity>
{
    static DeserializationResult deserialize(Entity& value, VectorView<const uint8_t> packet, size_t& offset)
    {
        if (auto res = Deserializer<Position>::deserialize(value.pos, packet, offset);
            res != DeserializationResult::Success) return res;
        if (auto res = Deserializer<std::string>::deserialize(value.name, packet, offset);
            res != DeserializationResult::Success) return res;
        return DeserializationResult::Success;
    }
};

struct ComplexCommandArgs
{
    Entity entity;
    SomeNote note;
};


PACKET_STRUCTURE(ComplexCommandArgs, &ComplexCommandArgs::entity, &ComplexCommandArgs::note)
