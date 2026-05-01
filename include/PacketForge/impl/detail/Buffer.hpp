#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>

namespace packet_forge {

struct Buffer
{
    std::vector<uint8_t> data;
    std::size_t valid_size = 0;
    static constexpr std::size_t SIZE = 16384;

    explicit Buffer(std::size_t capacity = SIZE) {
        data.resize(capacity); 
    }

    std::size_t capacity() const noexcept { return data.capacity(); }
};

using SharedBuffer = std::shared_ptr<Buffer>;

} // namespace packet_forge