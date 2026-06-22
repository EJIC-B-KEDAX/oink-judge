#include "oink_judge/test_node/verdict_utils.h"

#include <gtest/gtest.h>

#include <filesystem>

using namespace oink_judge::test_node;
namespace fs = std::filesystem;

class VerdictUtilsTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override { resources_ = fs::path("resources") / "test_verdict_utils"; }

    fs::path resources_; // NOLINT
};

TEST_F(VerdictUtilsTest, LoadAcceptedVerdictFromMeta) {
    const auto verdict = loadVerdictFromMeta("test1", (resources_ / "meta_ok.txt").string());

    ASSERT_NE(verdict, nullptr);
    EXPECT_EQ(verdict->getType().short_name, "AC");
    EXPECT_DOUBLE_EQ(verdict->getScore(), 100);
    EXPECT_DOUBLE_EQ(verdict->toJson(1)["time_used"].get<double>(), 1.5);
    EXPECT_DOUBLE_EQ(verdict->toJson(1)["memory_used"].get<double>(), 2048);
}

TEST_F(VerdictUtilsTest, LoadTimeLimitVerdictFromMeta) {
    const auto verdict = loadVerdictFromMeta("test1", (resources_ / "meta_time_limit.txt").string());

    ASSERT_NE(verdict, nullptr);
    EXPECT_EQ(verdict->getType().short_name, "TL");
    EXPECT_DOUBLE_EQ(verdict->getScore(), 0);
}

TEST_F(VerdictUtilsTest, LoadRuntimeErrorVerdictFromMeta) {
    const auto verdict = loadVerdictFromMeta("test1", (resources_ / "meta_runtime_error.txt").string());

    ASSERT_NE(verdict, nullptr);
    EXPECT_EQ(verdict->getType().short_name, "RE");
    EXPECT_DOUBLE_EQ(verdict->getScore(), 0);
}

TEST_F(VerdictUtilsTest, LoadAcceptedVerdictFromCheckerOutput) {
    const auto verdict = loadVerdictFromCheckerOutput("test1", (resources_ / "meta_ok.txt").string(),
                                                      (resources_ / "checker_accepted.txt").string());

    ASSERT_NE(verdict, nullptr);
    EXPECT_EQ(verdict->getType().short_name, "AC");
    EXPECT_DOUBLE_EQ(verdict->getScore(), 100);
}

TEST_F(VerdictUtilsTest, LoadWrongAnswerFromCheckerOutput) {
    const auto verdict = loadVerdictFromCheckerOutput("test1", (resources_ / "meta_ok.txt").string(),
                                                      (resources_ / "checker_wrong_answer.txt").string());

    ASSERT_NE(verdict, nullptr);
    EXPECT_EQ(verdict->getType().short_name, "WA");
    EXPECT_DOUBLE_EQ(verdict->getScore(), 0);
}

TEST_F(VerdictUtilsTest, LoadPresentationErrorFromCheckerOutput) {
    const auto verdict = loadVerdictFromCheckerOutput("test1", (resources_ / "meta_ok.txt").string(),
                                                      (resources_ / "checker_presentation_error.txt").string());

    ASSERT_NE(verdict, nullptr);
    EXPECT_EQ(verdict->getType().short_name, "PE");
    EXPECT_DOUBLE_EQ(verdict->getScore(), 0);
}

TEST_F(VerdictUtilsTest, CheckerOutputFallsBackToMetaWhenRunFailed) {
    const auto verdict = loadVerdictFromCheckerOutput("test1", (resources_ / "meta_time_limit.txt").string(),
                                                      (resources_ / "checker_accepted.txt").string());

    ASSERT_NE(verdict, nullptr);
    EXPECT_EQ(verdict->getType().short_name, "TL");
}
