#pragma once

#include <PacketForge/impl/detail/CommandType.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace packet_forge {

template <typename Tag>
class HeaderRepositoryNode
{
    using SuitType = CommandType<Tag>;
public:
    std::optional<SuitType> command;
    std::vector<std::pair<uint8_t, std::unique_ptr<HeaderRepositoryNode>>> children;

    explicit HeaderRepositoryNode(std::optional<SuitType> cmd = std::nullopt) noexcept
        : command(std::move(cmd)) {}

    bool isTerminal() const noexcept { return command.has_value(); }
};

} // namespace packet_forge
