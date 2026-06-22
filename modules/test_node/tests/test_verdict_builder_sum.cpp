#include "oink_judge/test_node/verdict_builders/verdict_builder_sum.h"

#include <gtest/gtest.h>

using namespace oink_judge::test_node;

namespace {

auto makeVerdict(const std::string& test_name, const VerdictType& type, double score, double time_used = 0,
                 double memory_used = 0) -> std::shared_ptr<VerdictBase> {
    auto verdict = std::make_shared<VerdictBase>(test_name);
    verdict->setInfo({.type = type, .score = score, .time_used = time_used, .memory_used = memory_used, .real_time_used = 0});
    return verdict;
}

} // namespace

class VerdictBuilderSumTest : public ::testing::Test {
  protected:
    static auto SetUpTestSuite() -> void { registerVerdictBuilderSumType(); }
};

TEST_F(VerdictBuilderSumTest, ClearResetsToAcceptedWithZeroScore) {
    VerdictBuilderSum builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::ACCEPTED, 50)); // NOLINT
    builder.clear();

    const auto result = builder.finalize();
    EXPECT_EQ(result->getType().short_name, "AC");
    EXPECT_DOUBLE_EQ(result->getScore(), 0);
}

TEST_F(VerdictBuilderSumTest, SumsScoresAcrossAcceptedVerdicts) {
    VerdictBuilderSum builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::ACCEPTED, 40, 1.0, 100)); // NOLINT
    builder.addVerdict(makeVerdict("sub2", VerdictType::ACCEPTED, 30, 3.0, 300)); // NOLINT

    const auto result = builder.finalize();
    EXPECT_DOUBLE_EQ(result->getScore(), 70);
    EXPECT_DOUBLE_EQ(result->getInfo().time_used, 3.0);
    EXPECT_DOUBLE_EQ(result->getInfo().memory_used, 300);
}

TEST_F(VerdictBuilderSumTest, WrongAnswerAfterAcceptedBecomesPartial) {
    VerdictBuilderSum builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::ACCEPTED, 50)); // NOLINT
    builder.addVerdict(makeVerdict("sub2", VerdictType::WRONG_ANSWER, 0));

    const auto result = builder.finalize();
    EXPECT_EQ(result->getType().short_name, "PT");
    EXPECT_DOUBLE_EQ(result->getScore(), 50);
}

TEST_F(VerdictBuilderSumTest, SkippedAfterAcceptedBecomesPartial) {
    VerdictBuilderSum builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::ACCEPTED, 25)); // NOLINT
    builder.addVerdict(makeVerdict("sub2", VerdictType::SKIPPED, 0));

    const auto result = builder.finalize();
    EXPECT_EQ(result->getType().short_name, "PT");
    EXPECT_DOUBLE_EQ(result->getScore(), 25);
}

TEST_F(VerdictBuilderSumTest, FailedOverridesPartial) {
    VerdictBuilderSum builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::ACCEPTED, 50)); // NOLINT
    builder.addVerdict(makeVerdict("sub2", VerdictType::WRONG_ANSWER, 0));
    builder.addVerdict(makeVerdict("sub3", VerdictType::FAILED, 0));

    const auto result = builder.finalize();
    EXPECT_EQ(result->getType().short_name, "FAIL");
}

TEST_F(VerdictBuilderSumTest, CanScoreChangeReflectsCurrentScore) {
    VerdictBuilderSum builder("group");

    EXPECT_FALSE(builder.canScoreChange());

    builder.addVerdict(makeVerdict("sub1", VerdictType::ACCEPTED, 10)); // NOLINT

    EXPECT_TRUE(builder.canScoreChange());
}

TEST_F(VerdictBuilderSumTest, FactoryCreatesRegisteredBuilder) {
    auto builder = VerdictBuilderFactory::instance().create("sum", "group");

    ASSERT_NE(builder, nullptr);
    const auto result = builder->finalize();
    EXPECT_EQ(result->getType().short_name, "AC");
    EXPECT_DOUBLE_EQ(result->getScore(), 0);
}
