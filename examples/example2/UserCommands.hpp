#include "UserCommandType.hpp"

#include "PacketForge/handlers/IntHandlers.hpp"
#include "PacketForge/handlers/StringHandlers.hpp"

struct Vector2D {
    uint32_t x;
    uint32_t y;
};

struct PlayerMoveData {
    uint32_t player_id;
    Vector2D position;
    uint8_t direction;
};

struct ChatMessageData {
    std::string sender;
    std::string message;
    uint32_t timestamp;
};

PACKET_STRUCTURE(Vector2D, &Vector2D::x, &Vector2D::y)
PACKET_STRUCTURE(PlayerMoveData, &PlayerMoveData::player_id, &PlayerMoveData::position, &PlayerMoveData::direction)
PACKET_STRUCTURE(ChatMessageData, &ChatMessageData::sender, &ChatMessageData::message, &ChatMessageData::timestamp)
