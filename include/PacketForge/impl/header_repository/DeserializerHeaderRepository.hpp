#pragma once

#include <PacketForge/impl/header_repository/HeaderRepositoryNode.hpp>
#include <PacketForge/impl/detail/vector_view/VectorView.hpp>

#include <algorithm>
#include <stdexcept>

namespace packet_forge {

template <typename Tag>
class DeserializerHeaderRepository
{
    using SuitType = CommandType<Tag>;
    using SuitNode = HeaderRepositoryNode<Tag>;

public:
    void addHeader(SuitType command, VectorView<const uint8_t> header)
    {
        if (header.empty()) {
            throw std::invalid_argument("Empty header is not allowed");
        }

        SuitNode* current = &root;
        for (uint8_t byte : header) {
            auto& ch = current->children;
            auto it = std::lower_bound(ch.begin(), ch.end(), byte,
                [](const auto& p, uint8_t v) { return p.first < v; });

            if (it != ch.end() && it->first == byte) {
                current = it->second.get();
                if (current->isTerminal()) {
                    throw std::logic_error("Conflict: new header extends an existing terminal header");
                }
            } else {
                auto new_node = std::make_unique<SuitNode>();
                SuitNode* new_node_ptr = new_node.get();
                ch.insert(it, {byte, std::move(new_node)});
                current = new_node_ptr;
            }
        }

        if (!current->children.empty()) {
            throw std::logic_error("Conflict: new header is a prefix of an existing longer header");
        }
        current->command = std::move(command);
    }

    SuitType getCommand(VectorView<const uint8_t> packet) const
    {
        const SuitNode* node = findNode(packet, true);
        return *node->command;
    }

    std::optional<SuitType> tryGetCommand(VectorView<const uint8_t> packet) const noexcept
    {
        if (const SuitNode* node = findNode(packet, false)) {
            return node->command;
        }
        return std::nullopt;
    }

private:
    const SuitNode* findNode(VectorView<const uint8_t> packet, bool throwOnError) const
    {
        const SuitNode* current = &root;
        for (uint8_t byte : packet) {
            auto it = std::lower_bound(current->children.begin(), current->children.end(), byte,
                [](const auto& p, uint8_t v) { return p.first < v; });

            if (it == current->children.end() || it->first != byte) {
                if (throwOnError) throw std::runtime_error("Header not found in repository");
                return nullptr;
            }

            current = it->second.get();
            if (current->isTerminal()) return current;
        }
        if (throwOnError) throw std::runtime_error("Incomplete header match");
        return nullptr;
    }

    SuitNode root;
};

} // namespace packet_forge
