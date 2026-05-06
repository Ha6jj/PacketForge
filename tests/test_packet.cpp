#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <atomic>

#include <PacketForge/impl/serialization/Packet.hpp>

namespace packet_forge {

struct MockSerializer : public IPacketSerializer {
    std::vector<uint8_t> payload;
    explicit MockSerializer(std::vector<uint8_t> p = {0x01, 0x02, 0x03}) 
        : payload(std::move(p)) {}
        
    void serialize(std::vector<uint8_t>& packet) const override {
        packet.insert(packet.end(), payload.begin(), payload.end());
    }
};

struct TagWithPool {};
struct TagWithoutPool {};

template<> struct CommandSuit<TagWithPool> {
    using type = uint16_t;
    static constexpr bool use_buffer_pool = true;
};
template<> struct CommandSuit<TagWithoutPool> {
    using type = uint16_t;
    static constexpr bool use_buffer_pool = false;
};

TEST(PacketTest, BuildWithPoolAppendsHeaderAndPayload) {
    auto pool = std::make_shared<SharedBufferPool>(4);
    std::vector<uint8_t> header_data = {0xAA, 0xBB};
    VectorView<const uint8_t> header(header_data);
    
    auto serializer = std::make_unique<MockSerializer>(std::vector<uint8_t>{0xDE, 0xAD});
    
    Packet<TagWithPool> pkt(std::move(serializer), header, pool);
    SharedBuffer buf = pkt.build();
    
    EXPECT_NE(buf.get(), nullptr);
    EXPECT_EQ(buf->valid_size, 4u);
    EXPECT_EQ(buf->data[0], 0xAA);
    EXPECT_EQ(buf->data[1], 0xBB);
    EXPECT_EQ(buf->data[2], 0xDE);
    EXPECT_EQ(buf->data[3], 0xAD);
    EXPECT_EQ(pool->total_overflow_allocs(), 0u);
}

TEST(PacketTest, BuildWithoutPoolReturnsVector) {
    std::vector<uint8_t> header_data = {0xCC};
    VectorView<const uint8_t> header(header_data);
    auto serializer = std::make_unique<MockSerializer>();
    
    Packet<TagWithoutPool> pkt(std::move(serializer), header, EmptyBufferPool{});
    std::vector<uint8_t> result = pkt.build();
    
    EXPECT_EQ(result.size(), 4u);
    EXPECT_EQ(result[0], 0xCC);
    EXPECT_EQ(result[1], 0x01);
    EXPECT_EQ(result[2], 0x02);
    EXPECT_EQ(result[3], 0x03);
}

TEST(PacketTest, NullSerializerBuildsHeaderOnly) {
    auto pool = std::make_shared<SharedBufferPool>(2);
    std::vector<uint8_t> header_data = {0x10, 0x20, 0x30};
    VectorView<const uint8_t> header(header_data);
    
    Packet<TagWithPool> pkt(nullptr, header, pool);
    SharedBuffer buf = pkt.build();
    
    EXPECT_EQ(buf->valid_size, 3u);
    EXPECT_EQ(buf->data[0], 0x10);
    EXPECT_EQ(buf->data[1], 0x20);
    EXPECT_EQ(buf->data[2], 0x30);
}

TEST(PacketTest, BufferDataIsClearedBeforeReuse) {
    auto pool = std::make_shared<SharedBufferPool>(1);
    
    auto dirty = pool->acquire();
    std::fill(dirty->data.begin(), dirty->data.end(), 0xFF);
    dirty->valid_size = dirty->data.size();
    dirty.reset();
    
    std::vector<uint8_t> header = {0x55};
    VectorView<const uint8_t> hv(header);
    Packet<TagWithPool> pkt(std::make_unique<MockSerializer>(std::vector<uint8_t>{0xAA}), hv, pool);
    
    SharedBuffer buf = pkt.build();
    
    EXPECT_EQ(buf->valid_size, 2u);
    EXPECT_EQ(buf->data.size(), 2u); 
    EXPECT_EQ(buf->data[0], 0x55);
    EXPECT_EQ(buf->data[1], 0xAA);
    
    for (size_t i = 0; i < buf->valid_size; ++i) {
        EXPECT_NE(buf->data[i], 0xFF) << "Dirty data leaked into valid_size region at index " << i;
    }
    EXPECT_EQ(pool->total_overflow_allocs(), 0u);
}

TEST(PacketTest, BuildReturnTypeDependsOnTag) {
    static_assert(std::is_same_v<decltype(std::declval<Packet<TagWithPool>>().build()), SharedBuffer>);
    static_assert(std::is_same_v<decltype(std::declval<Packet<TagWithoutPool>>().build()), std::vector<uint8_t>>);
    SUCCEED();
}

TEST(PacketTest, VectorViewLifetimeRequirement) {
    auto pool = std::make_shared<SharedBufferPool>(2);
    std::vector<uint8_t> payload_data = {0x01};
    std::vector<uint8_t> header_data = {0xF0, 0x0F};
    
    Packet<TagWithPool> pkt(
        std::make_unique<MockSerializer>(payload_data), 
        VectorView<const uint8_t>(header_data), 
        pool
    );
    
    auto buf = pkt.build();
    EXPECT_EQ(buf->valid_size, 3u);
    EXPECT_EQ(buf->data[0], 0xF0);
}

TEST(PacketTest, ConcurrentBuildsSharedPool) {
    constexpr int kThreads = 16;
    auto pool = std::make_shared<SharedBufferPool>(4); 
    
    std::vector<uint8_t> header = {0x77};
    VectorView<const uint8_t> hv(header);
    
    std::vector<std::thread> workers;
    std::atomic<int> success{0};
    
    for (int i = 0; i < kThreads; ++i) {
        workers.emplace_back([&, i]() {
            auto ser = std::make_unique<MockSerializer>(std::vector<uint8_t>{static_cast<uint8_t>(i)});
            Packet<TagWithPool> pkt(std::move(ser), hv, pool);
            
            SharedBuffer buf = pkt.build();
            EXPECT_EQ(buf->data[0], 0x77);
            EXPECT_EQ(buf->valid_size, 2u);
            success.fetch_add(1, std::memory_order_relaxed);
        });
    }
    
    for (auto& t : workers) t.join();
    EXPECT_EQ(success.load(), kThreads);
    
    EXPECT_LE(pool->total_overflow_allocs(), static_cast<size_t>(kThreads));
}

TEST(PacketTest, PacketMoveSemanticsWork) {
    auto pool = std::make_shared<SharedBufferPool>(2);
    std::vector<uint8_t> hdr = {0x11};
    VectorView<const uint8_t> hv(hdr);
    
    Packet<TagWithPool> p1(std::make_unique<MockSerializer>(), hv, pool);
    Packet<TagWithPool> p2(std::move(p1));
    
    EXPECT_EQ(p2.build()->valid_size, 4u);
    
    Packet<TagWithPool> p3(std::make_unique<MockSerializer>(std::vector<uint8_t>{0x99}), hv, pool);
    auto buf = p3.build();
    EXPECT_EQ(buf->data[0], 0x11);
    EXPECT_EQ(buf->data[1], 0x99);
}

} // namespace packet_forge
