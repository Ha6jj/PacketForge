#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include "../command_type.hpp"

struct HeaderRepositoryNode
{
    std::optional<CommandType> command;
    std::unordered_map<uint8_t, std::unique_ptr<HeaderRepositoryNode>> children;

    explicit HeaderRepositoryNode(std::optional<CommandType> cmd = std::nullopt) noexcept;
    bool isTerminal() const noexcept;
};