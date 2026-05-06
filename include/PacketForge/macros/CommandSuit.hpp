#pragma once

#include <PacketForge/impl/detail/CommandType.hpp>

#include <cstdint>

namespace packet_forge {

struct DefaultConfig {
    static constexpr bool use_buffer_pool = false;
};

} // namespace packet_forge


#define DEFINE_COMMAND_SUIT(NAME, CFG, ...)                         \
                                                                    \
namespace packet_forge {                                            \
                                                                    \
struct NAME##_tag {};                                               \
                                                                    \
template <>                                                         \
struct CommandSuit<NAME##_tag> {                                    \
    enum class type : uint32_t {                                    \
        __VA_ARGS__                                                 \
    };                                                              \
                                                                    \
    static constexpr bool use_buffer_pool = CFG::use_buffer_pool;   \
};                                                                  \
                                                                    \
} // namespace packet_forge

#define DEFINE_DEFAULT_COMMAND_SUIT(NAME, ...)                      \
    DEFINE_COMMAND_SUIT(NAME, DefaultConfig, __VA_ARGS__)
