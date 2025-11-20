#pragma once

#include <vector>
#include <stdexcept>
#include "header_repository_node.hpp"

class DeserializerHeaderRepository
{
public:
    void addHeader(CommandType command, const std::vector<uint8_t>& header);
    CommandType getCommand(const std::vector<uint8_t>& packet) const;

private:
    HeaderRepositoryNode root;
};