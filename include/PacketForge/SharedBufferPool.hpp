#pragma once

#include "impl/MPMCQueue.h"
#include <PacketForge/impl/detail/Buffer.hpp>

#include <vector>
#include <atomic>

namespace packet_forge {

class SharedBufferPool : public std::enable_shared_from_this<SharedBufferPool>
{
public:
    explicit SharedBufferPool(size_t cap = 4096)
        : queue_(cap)
    {
        std::vector<std::unique_ptr<Buffer>> temp;
        temp.reserve(cap);

        for (size_t i = 0; i < cap; ++i) temp.emplace_back(std::make_unique<Buffer>());
        for (auto& buf : temp) queue_.push(buf.release());
    }

    SharedBufferPool(const SharedBufferPool&) = delete;
    SharedBufferPool& operator=(const SharedBufferPool&) = delete;

    SharedBuffer acquire()
    {
        Buffer* raw = nullptr;
        if (queue_.try_pop(raw))
        {
            return wrap(raw);
        }

        size_t new_overflow = overflow_.fetch_add(1, std::memory_order_relaxed) + 1;
        return wrap(new Buffer());
    }

    size_t total_overflow_allocs() const noexcept { return overflow_.load(std::memory_order_relaxed); }

private:
    SharedBuffer wrap(Buffer* raw)
    {
        auto weak_self = weak_from_this();
        return std::shared_ptr<Buffer>(raw, [weak_self](Buffer* b) noexcept {
            if (auto self = weak_self.lock())
            {
                if (!self->queue_.try_push(b)) delete b;
            }
            else
            {
                delete b;
            }
        });
    }

    rigtorp::MPMCQueue<Buffer*> queue_;
    std::atomic<size_t> overflow_{0};
};

} // namespace packet_forge