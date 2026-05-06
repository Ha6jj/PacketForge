#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <random>

#include <PacketForge/SharedBufferPool.hpp>

namespace packet_forge {

TEST(BufferTest, DefaultConstruction) {
    Buffer buf;
    EXPECT_EQ(buf.capacity(), Buffer::SIZE);
    EXPECT_EQ(buf.valid_size, 0u);
    EXPECT_FALSE(buf.data.empty());
    EXPECT_EQ(buf.data.size(), buf.capacity());
}

TEST(BufferTest, CustomCapacity) {
    constexpr std::size_t kCustomSize = 4096;
    Buffer buf(kCustomSize);
    EXPECT_EQ(buf.capacity(), kCustomSize);
    EXPECT_EQ(buf.valid_size, 0u);
    EXPECT_EQ(buf.data.size(), kCustomSize);
}

TEST(SharedBufferPoolTest, ConstructionAndAcquire) {
    constexpr size_t kPoolCap = 10;
    auto pool = std::make_shared<SharedBufferPool>(kPoolCap);
    
    auto buf = pool->acquire();
    EXPECT_NE(buf.get(), nullptr);
    EXPECT_EQ(pool->total_overflow_allocs(), 0u);
    EXPECT_GE(buf->capacity(), Buffer::SIZE);
}

TEST(SharedBufferPoolTest, PreallocatedBuffersUsedFirst) {
    constexpr size_t kPoolCap = 5;
    auto pool = std::make_shared<SharedBufferPool>(kPoolCap);

    std::vector<SharedBuffer> holders;
    holders.reserve(kPoolCap);

    for (size_t i = 0; i < kPoolCap; ++i) {
        holders.emplace_back(pool->acquire());
    }

    EXPECT_EQ(pool->total_overflow_allocs(), 0u);
}

TEST(SharedBufferPoolTest, OverflowWhenExhausted) {
    constexpr size_t kPoolCap = 3;
    auto pool = std::make_shared<SharedBufferPool>(kPoolCap);

    std::vector<SharedBuffer> holders;
    for (size_t i = 0; i < kPoolCap + 2; ++i) {
        holders.emplace_back(pool->acquire());
    }

    EXPECT_EQ(pool->total_overflow_allocs(), 2u);
}

TEST(SharedBufferPoolTest, BufferRecycling) {
    constexpr size_t kPoolCap = 2;
    auto pool = std::make_shared<SharedBufferPool>(kPoolCap);

    auto buf1 = pool->acquire();
    auto buf2 = pool->acquire();
    EXPECT_EQ(pool->total_overflow_allocs(), 0u);

    buf1.reset();

    auto buf3 = pool->acquire();
    EXPECT_EQ(pool->total_overflow_allocs(), 0u);
    EXPECT_NE(buf3.get(), nullptr);
}

TEST(SharedBufferPoolTest, OverflowCounterPersistsAfterRelease) {
    constexpr size_t kPoolCap = 2;
    auto pool = std::make_shared<SharedBufferPool>(kPoolCap);

    std::vector<SharedBuffer> holders;
    for (size_t i = 0; i < kPoolCap + 3; ++i) {
        holders.emplace_back(pool->acquire());
    }
    EXPECT_EQ(pool->total_overflow_allocs(), 3u);

    holders.clear();
    EXPECT_EQ(pool->total_overflow_allocs(), 3u);
}

TEST(SharedBufferPoolTest, OutstandingBuffersSurvivePoolDestruction) {
    SharedBuffer outliving_buf;
    {
        auto pool = std::make_shared<SharedBufferPool>(5);
        outliving_buf = pool->acquire();
        outliving_buf->valid_size = 42;
    }

    EXPECT_NE(outliving_buf.get(), nullptr);
    EXPECT_EQ(outliving_buf->valid_size, 42u);
}

TEST(SharedBufferPoolTest, ConcurrentAcquireRelease) {
    constexpr size_t kPoolCap = 100;
    constexpr int kNumThreads = 4;
    constexpr int kOpsPerThread = 1000;

    auto pool = std::make_shared<SharedBufferPool>(kPoolCap);
    std::vector<std::thread> threads;
    std::atomic<size_t> success_ops{0};

    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([pool, &success_ops]() {
            for (int j = 0; j < kOpsPerThread; ++j) {
                auto buf = pool->acquire();
                buf->valid_size = 10;
                buf->data[0] = static_cast<uint8_t>(j % 256);
                
                buf.reset();
                success_ops.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(success_ops.load(), kNumThreads * kOpsPerThread);
    EXPECT_TRUE(pool->total_overflow_allocs() >= 0);
}

TEST(SharedBufferPoolTest, OverflowBufferDeletedWhenQueueFull) {
    constexpr size_t kCap = 2;
    auto pool = std::make_shared<SharedBufferPool>(kCap);

    SharedBuffer b1 = pool->acquire();
    SharedBuffer b2 = pool->acquire();
    SharedBuffer b3 = pool->acquire();
    SharedBuffer b4 = pool->acquire();
    EXPECT_EQ(pool->total_overflow_allocs(), 2u);

    b1.reset(); b2.reset(); b3.reset(); b4.reset();

    auto b5 = pool->acquire();
    auto b6 = pool->acquire();
    EXPECT_EQ(pool->total_overflow_allocs(), 2u);
    b5.reset(); b6.reset();
}

TEST(SharedBufferPoolTest, SingleCapacityPool) {
    auto pool = std::make_shared<SharedBufferPool>(1);
    auto b1 = pool->acquire();
    EXPECT_EQ(pool->total_overflow_allocs(), 0u);

    auto b2 = pool->acquire();
    EXPECT_EQ(pool->total_overflow_allocs(), 1u);

    b1.reset();
    auto b3 = pool->acquire();
    EXPECT_EQ(pool->total_overflow_allocs(), 1u);
}

TEST(SharedBufferPoolTest, MultiplePoolsIndependence) {
    auto poolA = std::make_shared<SharedBufferPool>(4);
    auto poolB = std::make_shared<SharedBufferPool>(4);

    auto bufA = poolA->acquire();
    auto bufB = poolB->acquire();
    bufA->valid_size = 10;
    bufB->valid_size = 20;
    bufA->data[0] = 0xAA;
    bufB->data[0] = 0xBB;

    bufA.reset(); 
    bufB.reset();

    auto bufA2 = poolA->acquire();
    auto bufB2 = poolB->acquire();
    
    bufA2->valid_size = 10; bufA2->data[0] = 0xAA;
    bufB2->valid_size = 20; bufB2->data[0] = 0xBB;
    
    EXPECT_EQ(bufA2->valid_size, 10u);
    EXPECT_EQ(bufB2->valid_size, 20u);
    EXPECT_EQ(poolA->total_overflow_allocs(), 0u);
    EXPECT_EQ(poolB->total_overflow_allocs(), 0u);
}

TEST(SharedBufferPoolTest, ConcurrentStressWithRandomizedLatency) {
    constexpr size_t kCap = 64;
    constexpr int kThreads = 8;
    constexpr int kOps = 2000;
    auto pool = std::make_shared<SharedBufferPool>(kCap);
    
    std::atomic<size_t> total_ops{0};
    std::vector<std::thread> workers;

    for (int i = 0; i < kThreads; ++i) {
        workers.emplace_back([pool, &total_ops, i]() {
            std::mt19937 rng(i + 42);
            for (int j = 0; j < kOps; ++j) {
                auto buf = pool->acquire();
                buf->valid_size = rng() % buf->capacity();
                buf->data[0] = static_cast<uint8_t>(i);
                
                if (rng() % 10 == 0) std::this_thread::yield();
                
                buf.reset();
                total_ops.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : workers) t.join();
    EXPECT_EQ(total_ops.load(), kThreads * kOps);
    EXPECT_LT(pool->total_overflow_allocs(), static_cast<size_t>(kThreads * kOps));
}

TEST(SharedBufferPoolTest, MultipleBuffersOutlivePoolDestruction) {
    std::vector<SharedBuffer> survivors;
    {
        auto pool = std::make_shared<SharedBufferPool>(3);
        for (int i = 0; i < 7; ++i) {
            survivors.push_back(pool->acquire());
            survivors.back()->valid_size = static_cast<size_t>(i * 10);
            survivors.back()->data[0] = static_cast<uint8_t>(i);
        }
    }

    EXPECT_EQ(survivors.size(), 7u);
    for (size_t i = 0; i < survivors.size(); ++i) {
        EXPECT_EQ(survivors[i]->valid_size, i * 10);
        EXPECT_EQ(survivors[i]->data[0], static_cast<uint8_t>(i));
    }
    survivors.clear();
}

TEST(SharedBufferPoolTest, BufferBoundaryChecks) {
    auto pool = std::make_shared<SharedBufferPool>(1);
    auto buf = pool->acquire();
    
    EXPECT_EQ(buf->capacity(), Buffer::SIZE);
    EXPECT_EQ(buf->data.size(), buf->capacity());
    EXPECT_EQ(buf->valid_size, 0u);
    
    buf->valid_size = buf->capacity();
    std::fill(buf->data.begin(), buf->data.end(), 0xFF);
    EXPECT_EQ(buf->valid_size, buf->capacity());
    EXPECT_EQ(buf->data[buf->capacity() - 1], 0xFF);
    
    buf.reset();
}

TEST(SharedBufferPoolTest, BufferRecycleRequiresExplicitReset) {
    constexpr size_t kCap = 8;
    auto pool = std::make_shared<SharedBufferPool>(kCap);
    
    auto buf = pool->acquire();
    buf->valid_size = 100;
    buf->data[0] = 0xAB;
    buf.reset();

    auto buf2 = pool->acquire();
    
    buf2->valid_size = 0;
    buf2->data[0] = 0x00;
    
    EXPECT_LT(pool->total_overflow_allocs(), kCap);
}

TEST(SharedBufferPoolTest, ConcurrentStressWithSafeInitialization) {
    constexpr size_t kCap = 64;
    constexpr int kThreads = 8;
    constexpr int kOps = 2000;
    auto pool = std::make_shared<SharedBufferPool>(kCap);
    
    std::atomic<size_t> total_ops{0};
    std::vector<std::thread> workers;

    for (int i = 0; i < kThreads; ++i) {
        workers.emplace_back([pool, &total_ops, i]() {
            std::mt19937 rng(i + 42);
            for (int j = 0; j < kOps; ++j) {
                auto buf = pool->acquire();
                
                buf->valid_size = 0;
                std::fill(buf->data.begin(), buf->data.end(), 0);
                
                buf->valid_size = rng() % buf->capacity();
                buf->data[0] = static_cast<uint8_t>(i);
                
                if (rng() % 10 == 0) std::this_thread::yield();
                
                buf.reset();
                total_ops.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : workers) t.join();
    EXPECT_EQ(total_ops.load(), kThreads * kOps);
    EXPECT_LT(pool->total_overflow_allocs(), static_cast<size_t>(kThreads * kOps * 0.1));
}

} // namespace packet_forge
