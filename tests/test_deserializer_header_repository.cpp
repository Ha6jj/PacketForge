#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstdint>

#include <PacketForge/impl/header_repository/DeserializerHeaderRepository.hpp>

struct TestTag {};
namespace packet_forge {
template <>
struct CommandSuit<TestTag> {
    using type = std::string;
};
}

using TestRepo = packet_forge::DeserializerHeaderRepository<TestTag>;

TEST(DeserializerHeaderRepositoryTest, BasicAddAndExactMatch) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01, 0x02};
    ASSERT_NO_THROW(repo.addHeader("CMD_A", hdr));

    const uint8_t pkt[] = {0x01, 0x02};
    EXPECT_EQ(repo.getCommand(pkt), "CMD_A");
}

TEST(DeserializerHeaderRepositoryTest, MatchWithTrailingPayload) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01, 0x02};
    repo.addHeader("CMD_A", hdr);

    const uint8_t pkt[] = {0x01, 0x02, 0xFF, 0xAA};
    EXPECT_EQ(repo.getCommand(pkt), "CMD_A");
}

TEST(DeserializerHeaderRepositoryTest, PrefixHeadersBothOrders) {
    TestRepo repo;
    const uint8_t short_hdr[] = {0x01};
    const uint8_t long_hdr[]  = {0x02, 0x02};

    ASSERT_NO_THROW(repo.addHeader("SHORT", short_hdr));
    ASSERT_NO_THROW(repo.addHeader("LONG", long_hdr));

    const uint8_t pkt_s[] = {0x01, 0x03};
    EXPECT_EQ(repo.getCommand(pkt_s), "SHORT");

    const uint8_t pkt_l[] = {0x02, 0x02, 0x04};
    EXPECT_EQ(repo.getCommand(pkt_l), "LONG");
}

TEST(DeserializerHeaderRepositoryTest, Conflict_NewHeaderExtendsTerminal) {
    TestRepo repo;
    const uint8_t existing[] = {0x01, 0x02};
    repo.addHeader("EXISTING", existing);

    const uint8_t conflicting[] = {0x01, 0x02, 0x03};
    EXPECT_THROW(repo.addHeader("NEW", conflicting), std::logic_error);
}

TEST(DeserializerHeaderRepositoryTest, Conflict_NewHeaderIsPrefix) {
    TestRepo repo;
    const uint8_t existing[] = {0x01, 0x02, 0x03};
    repo.addHeader("EXISTING", existing);

    const uint8_t conflicting[] = {0x01, 0x02};
    EXPECT_THROW(repo.addHeader("PREFIX", conflicting), std::logic_error);
}

TEST(DeserializerHeaderRepositoryTest, ExactDuplicateThrows) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01, 0x02};
    repo.addHeader("FIRST", hdr);

    EXPECT_THROW(repo.addHeader("SECOND", hdr), std::logic_error);
}

TEST(DeserializerHeaderRepositoryTest, EmptyHeaderThrowsInvalidArgument) {
    TestRepo repo;
    std::array<uint8_t, 0> empty_arr{};
    EXPECT_THROW(repo.addHeader("BAD", empty_arr), std::invalid_argument);
}

TEST(DeserializerHeaderRepositoryTest, GetCommandThrowsOnMismatch) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01, 0x02};
    repo.addHeader("CMD", hdr);

    const uint8_t pkt_wrong[] = {0x01, 0x03};
    EXPECT_THROW(repo.getCommand(pkt_wrong), std::runtime_error);
}

TEST(DeserializerHeaderRepositoryTest, GetCommandThrowsOnIncompletePacket) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01, 0x02};
    repo.addHeader("CMD", hdr);

    const uint8_t pkt_short[] = {0x01};
    EXPECT_THROW(repo.getCommand(pkt_short), std::runtime_error);
}

TEST(DeserializerHeaderRepositoryTest, TryGetCommandReturnsNulloptOnFailure) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01, 0x02};
    repo.addHeader("CMD", hdr);

    const uint8_t pkt_wrong[] = {0x02, 0x03};
    EXPECT_FALSE(repo.tryGetCommand(pkt_wrong).has_value());

    const uint8_t pkt_short[] = {0x01};
    EXPECT_FALSE(repo.tryGetCommand(pkt_short).has_value());
}

TEST(DeserializerHeaderRepositoryTest, TryGetCommandSuccess) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01, 0x02};
    repo.addHeader("CMD", hdr);

    const uint8_t pkt[] = {0x01, 0x02, 0xAA};
    auto res = repo.tryGetCommand(pkt);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, "CMD");
}

TEST(DeserializerHeaderRepositoryTest, BranchingHeaders) {
    TestRepo repo;
    const uint8_t h1[] = {0x10, 0x20};
    const uint8_t h2[] = {0x10, 0x30};
    
    repo.addHeader("BRANCH_A", h1);
    repo.addHeader("BRANCH_B", h2);

    const uint8_t pkt_a[] = {0x10, 0x20, 0x00};
    const uint8_t pkt_b[] = {0x10, 0x30, 0x00};
    
    EXPECT_EQ(repo.getCommand(pkt_a), "BRANCH_A");
    EXPECT_EQ(repo.getCommand(pkt_b), "BRANCH_B");
}

struct HeaderParam {
    std::string name;
    std::vector<uint8_t> header;
    std::vector<uint8_t> packet;
    std::string expected_cmd;
    bool should_match;
};

class DeserializerParamTest : public ::testing::TestWithParam<HeaderParam> {
protected:
    TestRepo repo;
    void SetUp() override {
        const auto& p = GetParam();
        if (!p.header.empty()) {
            ASSERT_NO_THROW(repo.addHeader(p.name + "_HDR", p.header));
        }
    }
};

TEST_P(DeserializerParamTest, MatchAndMismatch) {
    const auto& param = GetParam();
    if (param.header.empty()) return;
    
    if (param.should_match) {
        EXPECT_EQ(repo.getCommand(param.packet), param.name + "_HDR");
        ASSERT_TRUE(repo.tryGetCommand(param.packet).has_value());
    } else {
        EXPECT_THROW(repo.getCommand(param.packet), std::runtime_error);
        EXPECT_FALSE(repo.tryGetCommand(param.packet).has_value());
    }
}

INSTANTIATE_TEST_SUITE_P(
    VariousPatterns,
    DeserializerParamTest,
    ::testing::Values(
        HeaderParam{"A", {0xAA}, {0xAA, 0x11}, "A", true},
        HeaderParam{"B", {0xBB, 0x22}, {0xBB, 0x22, 0x33}, "B", true},
        HeaderParam{"C", {0xCC, 0xDD}, {0xCC, 0xEE}, "C", false},
        HeaderParam{"D", {0x01, 0x02, 0x03}, {0x01, 0x02}, "D", false}
    )
);

TEST(DeserializerHeaderRepositoryTest, StdVectorAndArrayIntegration) {
    TestRepo repo;
    
    std::vector<uint8_t> hdr_vec = {0x10, 0x20, 0x30};
    std::array<uint8_t, 2> hdr_arr = {0x40, 0x50};
    
    ASSERT_NO_THROW(repo.addHeader("VEC_CMD", hdr_vec));
    ASSERT_NO_THROW(repo.addHeader("ARR_CMD", hdr_arr));
    
    std::vector<uint8_t> pkt_vec = {0x10, 0x20, 0x30, 0xFF};
    std::array<uint8_t, 4> pkt_arr = {0x40, 0x50, 0x00, 0x01};
    
    EXPECT_EQ(repo.getCommand(pkt_vec), "VEC_CMD");
    EXPECT_EQ(repo.getCommand(pkt_arr), "ARR_CMD");
}

TEST(DeserializerHeaderRepositoryTest, MoveSemanticsForCommandType) {
    TestRepo repo;
    std::string heavy_cmd(1024, 'X');
    const uint8_t hdr[] = {0xDE, 0xAD};
    
    ASSERT_NO_THROW(repo.addHeader(std::move(heavy_cmd), hdr));
    
    const uint8_t pkt[] = {0xDE, 0xAD, 0xBE, 0xEF};
    EXPECT_EQ(repo.getCommand(pkt), std::string(1024, 'X'));
}

TEST(DeserializerHeaderRepositoryTest, NullByteInHeader) {
    TestRepo repo;
    const uint8_t hdr[] = {0x00, 0x01, 0x00};
    ASSERT_NO_THROW(repo.addHeader("NULL_HDR", hdr));
    
    const uint8_t pkt[] = {0x00, 0x01, 0x00, 0xFF};
    EXPECT_EQ(repo.getCommand(pkt), "NULL_HDR");
}

TEST(DeserializerHeaderRepositoryTest, DeepTreeWithSharedPrefixes) {
    TestRepo repo;
    const uint8_t h1[] = {0x01, 0x02, 0x03, 0x04};
    const uint8_t h2[] = {0x01, 0x02, 0x05, 0x06};
    const uint8_t h3[] = {0x01, 0x07, 0x08};
    
    repo.addHeader("DEEP_A", h1);
    repo.addHeader("DEEP_B", h2);
    repo.addHeader("DEEP_C", h3);
    
    const uint8_t pkt_a[] = {0x01, 0x02, 0x03, 0x04, 0xFF};
    const uint8_t pkt_b[] = {0x01, 0x02, 0x05, 0x06, 0xFF};
    const uint8_t pkt_c[] = {0x01, 0x07, 0x08, 0xFF};
    const uint8_t pkt_fail[] = {0x01, 0x02, 0x09, 0x0A};
    
    EXPECT_EQ(repo.getCommand(pkt_a), "DEEP_A");
    EXPECT_EQ(repo.getCommand(pkt_b), "DEEP_B");
    EXPECT_EQ(repo.getCommand(pkt_c), "DEEP_C");
    EXPECT_FALSE(repo.tryGetCommand(pkt_fail).has_value());
}

TEST(DeserializerHeaderRepositoryTest, WideTreeManySiblings) {
    TestRepo repo;
    for (int i = 0; i < 20; ++i) {
        uint8_t hdr[] = {0xAA, static_cast<uint8_t>(i)};
        ASSERT_NO_THROW(repo.addHeader("WIDE_" + std::to_string(i), hdr));
    }
    
    for (int i = 0; i < 20; ++i) {
        uint8_t pkt[] = {0xAA, static_cast<uint8_t>(i), 0x00};
        EXPECT_EQ(repo.getCommand(pkt), "WIDE_" + std::to_string(i));
    }
}

TEST(DeserializerHeaderRepositoryTest, TryGetCommandIsNoexcept) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01};
    repo.addHeader("CMD", hdr);
    
    const uint8_t pkt_match[] = {0x01, 0x02};
    const uint8_t pkt_fail[]  = {0x02, 0x03};
    
    static_assert(noexcept(repo.tryGetCommand(pkt_match)), 
                  "tryGetCommand must be noexcept");
    static_assert(noexcept(repo.tryGetCommand(pkt_fail)), 
                  "tryGetCommand must be noexcept even on failure");
                  
    EXPECT_TRUE(repo.tryGetCommand(pkt_match).has_value());
    EXPECT_FALSE(repo.tryGetCommand(pkt_fail).has_value());
}

TEST(DeserializerHeaderRepositoryTest, ExceptionSafetyStatePreservation) {
    TestRepo repo;
    const uint8_t valid_hdr[] = {0x10, 0x20};
    repo.addHeader("VALID", valid_hdr);
    
    const uint8_t conflict_hdr[] = {0x10, 0x20, 0x30};
    EXPECT_THROW(repo.addHeader("CONFLICT", conflict_hdr), std::logic_error);
    
    const uint8_t pkt[] = {0x10, 0x20, 0xFF};
    const uint8_t wrong_pkt[] = {0xFF, 0xFE};
    
    EXPECT_EQ(repo.getCommand(pkt), "VALID");
    EXPECT_FALSE(repo.tryGetCommand(wrong_pkt).has_value());
}

TEST(DeserializerHeaderRepositoryTest, GetCommandAndTryGetCommandConsistency) {
    TestRepo repo;
    const uint8_t hdr[] = {0xAB, 0xCD};
    repo.addHeader("TEST_CMD", hdr);
    
    std::vector<std::vector<uint8_t>> packets = {
        {0xAB, 0xCD},
        {0xAB, 0xCD, 0x00, 0xFF},
        {0xAB, 0xCE},
        {0xAB},
        {0xFF, 0xAA}
    };
    
    for (const auto& pkt : packets) {
        auto opt_res = repo.tryGetCommand(pkt);
        bool has_val = opt_res.has_value();
        
        if (has_val) {
            EXPECT_EQ(repo.getCommand(pkt), *opt_res);
        } else {
            EXPECT_THROW(repo.getCommand(pkt), std::runtime_error);
        }
    }
}

TEST(DeserializerHeaderRepositoryTest, ConstCorrectness) {
    TestRepo repo;
    const uint8_t hdr[] = {0x01, 0x02};
    repo.addHeader("CONST_TEST", hdr);
    
    const TestRepo& const_repo = repo;
    const uint8_t pkt[] = {0x01, 0x02, 0x00};
    
    EXPECT_EQ(const_repo.getCommand(pkt), "CONST_TEST");
    EXPECT_TRUE(const_repo.tryGetCommand(pkt).has_value());
}

TEST(DeserializerHeaderRepositoryTest, LargeNumberOfHeaders) {
    TestRepo repo;
    const size_t N = 1000;
    
    for (size_t i = 0; i < N; ++i) {
        uint8_t hdr[3] = {
            static_cast<uint8_t>(i & 0xFF),
            static_cast<uint8_t>((i >> 8) & 0xFF),
            static_cast<uint8_t>((i >> 16) & 0xFF)
        };
        ASSERT_NO_THROW(repo.addHeader("CMD_" + std::to_string(i), hdr));
    }
    
    for (size_t i = 0; i < N; i += 50) {
        uint8_t pkt[4] = {
            static_cast<uint8_t>(i & 0xFF),
            static_cast<uint8_t>((i >> 8) & 0xFF),
            static_cast<uint8_t>((i >> 16) & 0xFF),
            0xDE
        };
        EXPECT_EQ(repo.getCommand(pkt), "CMD_" + std::to_string(i));
    }
}