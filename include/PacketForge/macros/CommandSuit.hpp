#pragma once

#include "../impl/detail/CommandType.hpp"

#include <cstdint>

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
