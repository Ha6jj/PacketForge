#include "Serializer.hpp"

enum class CommandType : uint8_t
{
    COMPLEX_COMMAND
};

struct Position
{
    uint32_t x;
    uint32_t y;
};

template <>
struct Serializer<Position>
{
    static void serialize(const Position& value, std::vector<uint8_t>& packet)
    {
        Serializer<uint32_t>::serialize(value.x, packet);
        Serializer<uint32_t>::serialize(value.y, packet);
    }
};

template <>
struct Deserializer<Position>
{
    static void deserialize(Position& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset >= packet.size())
        {
            throw std::out_of_range("Packet too short");
        }
        Deserializer<uint32_t>::deserialize(value.x, packet, offset);
        Deserializer<uint32_t>::deserialize(value.y, packet, offset);
    }
};

struct SomeNote
{
    uint8_t system_flags;
    std::string note;
};

template <>
struct Serializer<SomeNote>
{
    static void serialize(const SomeNote& value, std::vector<uint8_t>& packet)
    {
        Serializer<uint8_t>::serialize(value.system_flags, packet);
        Serializer<std::string>::serialize(value.note, packet);
    }
};

template <>
struct Deserializer<SomeNote>
{
    static void deserialize(SomeNote& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset >= packet.size())
        {
            throw std::out_of_range("Packet too short");
        }
        Deserializer<uint8_t>::deserialize(value.system_flags, packet, offset);
        Deserializer<std::string>::deserialize(value.note, packet, offset);
    }
};

struct Entity
{
    Position pos;
    std::string name;
};

template <>
struct Serializer<Entity>
{
    static void serialize(const Entity& value, std::vector<uint8_t>& packet)
    {
        Serializer<Position>::serialize(value.pos, packet);
        Serializer<std::string>::serialize(value.name, packet);
    }
};

template <>
struct Deserializer<Entity>
{
    static void deserialize(Entity& value, const std::vector<uint8_t>& packet, size_t& offset)
    {
        if (offset >= packet.size())
        {
            throw std::out_of_range("Packet too short");
        }
        Deserializer<Position>::deserialize(value.pos, packet, offset);
        Deserializer<std::string>::deserialize(value.name, packet, offset);
    }
};

struct ComplexCommandArgs
{
    Entity entity;
    SomeNote note;
};


PACKET_STRUCTURE(ComplexCommandArgs, &ComplexCommandArgs::entity, &ComplexCommandArgs::note)
