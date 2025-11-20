#include "header_repository_node.hpp"

HeaderRepositoryNode::HeaderRepositoryNode(std::optional<CommandType> cmd) noexcept
    : command(std::move(cmd)) {}

bool HeaderRepositoryNode::isTerminal() const noexcept
{
    return command.has_value();
}