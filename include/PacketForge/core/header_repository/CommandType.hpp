#pragma once

#include <cstdint>

namespace packet_forge {

template <typename Tag>
struct CommandSuit;

template <typename Tag>
using CommandType = typename CommandSuit<Tag>::type;

} // namespace packet_forge
