#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <array>
#include <cstring>

#include <PacketForge/CommandFactory.hpp>
#include <PacketForge/macros/CommandSuit.hpp>
#include <PacketForge/macros/PacketStructure.hpp>
#include <PacketForge/serializers/IntSerializers.hpp>
#include <PacketForge/serializers/ArraySerializers.hpp>

struct TestPayloadSimple {
    uint32_t id = 0;
    uint8_t status = 0;
};
PACKET_STRUCTURE(TestPayloadSimple, &TestPayloadSimple::id, &TestPayloadSimple::status)

struct TestPayloadComplex {
    uint16_t version = 0;
    uint32_t length = 0;
    std::array<int, 4> flags = {0, 0, 0, 0};
    int32_t timestamp = 0;
};
PACKET_STRUCTURE(TestPayloadComplex, &TestPayloadComplex::version, &TestPayloadComplex::length,
                 &TestPayloadComplex::flags, &TestPayloadComplex::timestamp)

DEFINE_DEFAULT_COMMAND_SUIT(WithoutPool, WithoutPoolCommand)
DEFINE_DEFAULT_COMMAND_SUIT(Stream, StreamCommand)
DEFINE_DEFAULT_COMMAND_SUIT(Error, ErrorCommand)

struct WithPoolConfig : packet_forge::DefaultConfig {
    static constexpr bool use_buffer_pool = true;
};
DEFINE_COMMAND_SUIT(WithPool, WithPoolConfig, WithPoolCommand)

namespace {
template <typename Tag>
auto packetToBytes(const packet_forge::Packet<Tag>& pkt) {
    return pkt.build();
}

template <typename T, typename Tag>
T extractPayload(const packet_forge::PacketDescriptor<Tag>& desc) {
    auto& deserializer_ref = *desc.deserializer;
    return static_cast<packet_forge::PacketDeserializer<T>&>(deserializer_ref).getArgs();
}
}

namespace packet_forge {

TEST(CommandFactoryTest, Roundtrip_SimplePayload) {
    CommandFactory<WithoutPool_tag> factory;
    factory.registerCommand<TestPayloadSimple>(CommandType<WithoutPool_tag>::WithoutPoolCommand, {0xDE, 0xAD});

    TestPayloadSimple original{999, 0x05};
    auto packet = factory.create(CommandType<WithoutPool_tag>::WithoutPoolCommand, original);
    auto bytes = packetToBytes(packet);
    auto view = VectorView<const uint8_t>(bytes.data(), bytes.size());

    auto result = factory.deserializePacket(view);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->command, CommandType<WithoutPool_tag>::WithoutPoolCommand);

    auto restored = extractPayload<TestPayloadSimple, WithoutPool_tag>(*result);
    ASSERT_EQ(restored.id, 999);
    ASSERT_EQ(restored.status, 0x05);
}

TEST(CommandFactoryTest, Roundtrip_ComplexPayload) {
    CommandFactory<WithoutPool_tag> factory;
    factory.registerCommand<TestPayloadComplex>(CommandType<WithoutPool_tag>::WithoutPoolCommand, {0x10, 0x20});

    TestPayloadComplex original{0x0102, 0x0000ABCD, {0x11, 0x22, 0x33, 0x44}, -123456};
    auto packet = factory.create(CommandType<WithoutPool_tag>::WithoutPoolCommand, original);
    auto bytes = packetToBytes(packet);

    auto view = VectorView<const uint8_t>(bytes.data(), bytes.size());
    auto result = factory.deserializePacket(view);
    ASSERT_TRUE(result.has_value());

    auto restored = extractPayload<TestPayloadComplex, WithoutPool_tag>(*result);
    ASSERT_EQ(restored.version, original.version);
    ASSERT_EQ(restored.length, original.length);
    ASSERT_EQ(std::memcmp(restored.flags.data(), original.flags.data(), original.flags.size()), 0);
    ASSERT_EQ(restored.timestamp, original.timestamp);
}

TEST(CommandFactoryTest, DeserializeStream_MultiplePackets) {
    CommandFactory<Stream_tag> factory;
    factory.registerCommand<TestPayloadSimple>(CommandType<Stream_tag>::StreamCommand, {0xAA});

    std::vector<uint8_t> stream;
    for (uint32_t i = 1; i <= 5; ++i) {
        TestPayloadSimple p{i, static_cast<uint8_t>(i * 10)};
        auto pkt = factory.create(CommandType<Stream_tag>::StreamCommand, p);
        auto bytes = packetToBytes(pkt);
        stream.insert(stream.end(), bytes.begin(), bytes.end());
    }

    auto view = VectorView<const uint8_t>(stream.data(), stream.size());
    auto descriptors = factory.deserializeStream(view);

    ASSERT_EQ(descriptors.size(), 5);
    for (size_t i = 0; i < 5; ++i) {
        ASSERT_EQ(descriptors[i].command, CommandType<Stream_tag>::StreamCommand);
        auto payload = extractPayload<TestPayloadSimple, Stream_tag>(descriptors[i]);
        ASSERT_EQ(payload.id, static_cast<uint32_t>(i + 1));
        ASSERT_EQ(payload.status, static_cast<uint8_t>((i + 1) * 10));
    }
}

TEST(CommandFactoryTest, DeserializeStream_TruncatedTail) {
    CommandFactory<Stream_tag> factory;
    factory.registerCommand<TestPayloadSimple>(CommandType<Stream_tag>::StreamCommand, {0xBB});

    TestPayloadSimple p{10, 20};
    auto pkt = factory.create(CommandType<Stream_tag>::StreamCommand, p);
    auto bytes = packetToBytes(pkt);

    std::vector<uint8_t> stream = bytes;
    stream.push_back(0xFF);

    auto view = VectorView<const uint8_t>(stream.data(), stream.size());
    auto descriptors = factory.deserializeStream(view);

    ASSERT_EQ(descriptors.size(), 1);
    ASSERT_FALSE(view.empty());
}

TEST(CommandFactoryTest, PeekCommand_ValidAndEmpty) {
    CommandFactory<WithoutPool_tag> factory;
    factory.registerCommand<TestPayloadSimple>(CommandType<WithoutPool_tag>::WithoutPoolCommand, {0xDE, 0xAD});

    TestPayloadSimple p{1, 2};
    auto pkt = factory.create(CommandType<WithoutPool_tag>::WithoutPoolCommand, p);
    auto bytes = packetToBytes(pkt);
    auto view = VectorView<const uint8_t>(bytes.data(), bytes.size());

    auto cmd = factory.peekCommand(view);
    ASSERT_TRUE(cmd.has_value());
    ASSERT_EQ(*cmd, CommandType<WithoutPool_tag>::WithoutPoolCommand);

    VectorView<const uint8_t> empty_view;
    ASSERT_FALSE(factory.peekCommand(empty_view).has_value());
}

TEST(CommandFactoryTest, DeserializePacket_UnknownCommand) {
    CommandFactory<Error_tag> factory;
    factory.registerCommand<TestPayloadSimple>(CommandType<Error_tag>::ErrorCommand, {0xDD});

    std::vector<uint8_t> raw_bytes = {0xEE, 0xFF, 0x01, 0x02};
    auto view = VectorView<const uint8_t>(raw_bytes.data(), raw_bytes.size());

    EXPECT_THROW(factory.deserializePacket(view), std::runtime_error);
}

TEST(CommandFactoryTest, DeserializePacket_CorruptPayload) {
    CommandFactory<Error_tag> factory;
    factory.registerCommand<TestPayloadSimple>(CommandType<Error_tag>::ErrorCommand, {0xCC});

    TestPayloadSimple p{100, 50};
    auto pkt = factory.create(CommandType<Error_tag>::ErrorCommand, p);
    auto bytes = packetToBytes(pkt);

    bytes.pop_back();
    auto view = VectorView<const uint8_t>(bytes.data(), bytes.size());

    auto result = factory.deserializePacket(view);
    ASSERT_FALSE(result.has_value());
}

TEST(CommandFactoryTest, BufferPool_CreationAndRoundtrip) {
    auto pool = std::make_shared<SharedBufferPool>(64);
    CommandFactory<WithPool_tag> factory(pool);
    factory.registerCommand<TestPayloadSimple>(CommandType<WithPool_tag>::WithPoolCommand, {0x88});

    for (int i = 0; i < 15; ++i) {
        TestPayloadSimple p{static_cast<uint32_t>(i), static_cast<uint8_t>(i)};
        auto packet = factory.create(CommandType<WithPool_tag>::WithPoolCommand, p);
        auto bytes = packetToBytes(packet);
        ASSERT_FALSE(bytes->valid_size == 0);
    }

    TestPayloadSimple original{99, 0x99};
    auto pkt = factory.create(CommandType<WithPool_tag>::WithPoolCommand, original);
    SharedBuffer bytes = packetToBytes(pkt);
    auto view = VectorView<const uint8_t>(bytes->data);
    auto res = factory.deserializePacket(view);
    
    ASSERT_TRUE(res.has_value());
    auto restored = extractPayload<TestPayloadSimple, WithPool_tag>(*res);
    ASSERT_EQ(restored.id, 99);
}

TEST(CommandFactoryTest, Create_WithRvaluePayload) {
    CommandFactory<WithoutPool_tag> factory;
    factory.registerCommand<TestPayloadSimple>(CommandType<WithoutPool_tag>::WithoutPoolCommand, {0x05});

    auto packet = factory.create(CommandType<WithoutPool_tag>::WithoutPoolCommand, TestPayloadSimple{123, 0x7B});
    auto bytes = packetToBytes(packet);
    auto view = VectorView<const uint8_t>(bytes.data(), bytes.size());
    
    auto result = factory.deserializePacket(view);
    ASSERT_TRUE(result.has_value());
    auto restored = extractPayload<TestPayloadSimple, WithoutPool_tag>(*result);
    ASSERT_EQ(restored.id, 123);
    ASSERT_EQ(restored.status, 0x7B);
}

TEST(CommandFactoryTest, EdgeCases_EmptyAndZeroSized) {
    CommandFactory<Error_tag> factory;
    VectorView<const uint8_t> empty_view;
    
    auto result = factory.deserializePacket(empty_view);
    ASSERT_FALSE(result.has_value());

    ASSERT_FALSE(factory.peekCommand(empty_view).has_value());
}

} // namespace packet_forge