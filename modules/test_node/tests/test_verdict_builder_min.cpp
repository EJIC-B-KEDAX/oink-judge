#include "oink_judge/test_node/verdict_builders/verdict_builder_min.h"

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

class VerdictBuilderMinTest : public ::testing::Test {
  protected:
    static auto SetUpTestSuite() -> void { registerVerdictBuilderMinType(); }
};

TEST_F(VerdictBuilderMinTest, ClearResetsToAcceptedWithFullScore) {
    VerdictBuilderMin builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::WRONG_ANSWER, 0));
    builder.clear();

    const auto result = builder.finalize();
    EXPECT_EQ(result->getType().short_name, "AC");
    EXPECT_DOUBLE_EQ(result->getScore(), 100);
}

TEST_F(VerdictBuilderMinTest, TakesMinimumScoreAcrossVerdicts) {
    VerdictBuilderMin builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::ACCEPTED, 80, 1.0, 100)); // NOLINT
    builder.addVerdict(makeVerdict("sub2", VerdictType::ACCEPTED, 60, 2.0, 200)); // NOLINT

    const auto result = builder.finalize();
    EXPECT_DOUBLE_EQ(result->getScore(), 60);
    EXPECT_DOUBLE_EQ(result->getInfo().time_used, 2.0);
    EXPECT_DOUBLE_EQ(result->getInfo().memory_used, 200);
}

TEST_F(VerdictBuilderMinTest, WrongAnswerOverridesAcceptedType) {
    VerdictBuilderMin builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::WRONG_ANSWER, 0));

    const auto result = builder.finalize();
    EXPECT_EQ(result->getType().short_name, "WA");
    EXPECT_DOUBLE_EQ(result->getScore(), 0);
}

TEST_F(VerdictBuilderMinTest, FailedOverridesWrongAnswer) {
    VerdictBuilderMin builder("group");

    builder.addVerdict(makeVerdict("sub1", VerdictType::WRONG_ANSWER, 0));
    builder.addVerdict(makeVerdict("sub2", VerdictType::FAILED, 0));

    const auto result = builder.finalize();
    EXPECT_EQ(result->getType().short_name, "FAIL");
}

TEST_F(VerdictBuilderMinTest, CanScoreChangeReflectsCurrentScore) {
    VerdictBuilderMin builder("group");

    EXPECT_TRUE(builder.canScoreChange());

    builder.addVerdict(makeVerdict("sub1", VerdictType::WRONG_ANSWER, 0));

    EXPECT_FALSE(builder.canScoreChange());
}

TEST_F(VerdictBuilderMinTest, FactoryCreatesRegisteredBuilder) {
    auto builder = VerdictBuilderFactory::instance().create("min", "group");

    ASSERT_NE(builder, nullptr);
    const auto result = builder->finalize();
    EXPECT_EQ(result->getType().short_name, "AC");
}
