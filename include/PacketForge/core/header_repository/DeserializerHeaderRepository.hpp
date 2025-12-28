#pragma once

#include <vector>
#include <stdexcept>
#include "HeaderRepositoryNode.hpp"

namespace packet_forge {

template <typename Tag>
class DeserializerHeaderRepository
{
    using SuitType = CommandType<Tag>;
    using SuitNode = HeaderRepositoryNode<Tag>;
public:
    void addHeader(SuitType command, const std::vector<uint8_t>& header)
    {
        if (header.empty())
        {
            throw std::runtime_error("Empty header is not allowed");
        }

        SuitNode* current = &root;
        for (uint8_t byte : header)
        {
            auto& children = current->children;
            auto it = children.find(byte);

            if (it != children.end())
            {
                current = it->second.get();
                if (current->isTerminal())
                {
                    throw std::runtime_error("Header can't be a prefix of another");
                }
            }
            else
            {
                auto new_node = std::make_unique<SuitNode>();
                SuitNode* new_node_ptr = new_node.get();
                children[byte] = std::move(new_node);
                current = new_node_ptr;
            }
        }

        if (!current->children.empty())
        {
            throw std::runtime_error("Header can't be a prefix of another");
        }

        current->command = command;
    }

    SuitType getCommand(const std::vector<uint8_t>& packet) const
    {
        const SuitNode* current = &root;

        for (uint8_t byte : packet)
        {
            auto it = current->children.find(byte);
            if (it == current->children.end())
            {
                throw std::runtime_error("Header not found");
            }

            current = it->second.get();
            if (current->isTerminal())
            {
                return *current->command;
            }
        }

        throw std::runtime_error("Incomplete header");
    }

private:
    SuitNode root;
};

} // namespace packet_forge
