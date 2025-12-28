#pragma once

#include "core/header_repository/CommandType.hpp"
#include "core/Serializer.hpp"

#include <vector>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace packet_forge {

template <typename T>
using base_type = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename Struct, typename... Members>
void serialize_members(const Struct& value, std::vector<uint8_t>& packet, Members... members) {
    (Serializer<base_type<decltype(value.*members)>>::serialize(value.*members, packet), ...);
}

template <typename Struct, typename... Members>
void deserialize_members(Struct& value, const std::vector<uint8_t>& packet, size_t& offset, Members... members) {
    (Deserializer<base_type<decltype(value.*members)>>::deserialize(value.*members, packet, offset), ...);
}

} // namespace packet_forge


#define PACKET_STRUCTURE(struct_name, ...)                                          \
                                                                                    \
namespace packet_forge {                                                            \
                                                                                    \
template <>                                                                         \
struct Serializer<struct_name> {                                                    \
    static void serialize(const struct_name& value, std::vector<uint8_t>& packet) { \
        serialize_members(value, packet, __VA_ARGS__);                              \
    }                                                                               \
};                                                                                  \
                                                                                    \
template <>                                                                         \
struct Deserializer<struct_name> {                                                  \
    static void deserialize(struct_name& value,                                     \
                           const std::vector<uint8_t>& packet,                      \
                           size_t& offset) {                                        \
        deserialize_members(value, packet, offset, __VA_ARGS__);                    \
    }                                                                               \
};                                                                                  \
                                                                                    \
} // namespace packet_forge


#define DEFINE_COMMAND_SUIT(NAME, ...)  \
                                        \
namespace packet_forge {                \
                                        \
struct NAME##_tag {};                   \
                                        \
template <>                             \
struct CommandSuit<NAME##_tag> {        \
    enum class type : uint32_t {        \
        __VA_ARGS__                     \
    };                                  \
};                                      \
                                        \
} // namespace packet_forge
