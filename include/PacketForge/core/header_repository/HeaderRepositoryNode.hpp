#pragma once

#include "CommandType.hpp"

#include <memory>
#include <optional>
#include <unordered_map>

namespace packet_forge {

struct HeaderRepositoryNode
{
    std::optional<CommandType> command;
    std::unordered_map<uint8_t, std::unique_ptr<HeaderRepositoryNode>> children;

    explicit HeaderRepositoryNode(std::optional<CommandType> cmd = std::nullopt) noexcept
        : command(std::move(cmd)) {}

    bool isTerminal() const noexcept
    {
        return command.has_value();
    }
};

} // namespace packet_forge
