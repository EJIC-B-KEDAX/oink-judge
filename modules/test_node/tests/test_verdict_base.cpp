#include "oink_judge/test_node/verdicts/verdict_base.h"

#include <gtest/gtest.h>

using namespace oink_judge::test_node;

TEST(VerdictBaseTest, DefaultInfoIsEmptyVerdict) {
    VerdictBase verdict("test1");

    EXPECT_EQ(verdict.getType().short_name, "EM");
    EXPECT_DOUBLE_EQ(verdict.getScore(), 0.0);
}

TEST(VerdictBaseTest, ToJsonIncludesVerdictAndScore) {
    VerdictBase verdict("test1");
    verdict.setInfo(
        {.type = VerdictType::ACCEPTED, .score = 100, .time_used = 1.5, .memory_used = 2048, .real_time_used = 2.0}); // NOLINT

    const auto json = verdict.toJson(0);

    EXPECT_EQ(json["verdict"]["short_name"], "AC");
    EXPECT_EQ(json["verdict"]["full_name"], "Accepted");
    EXPECT_DOUBLE_EQ(json["score"].get<double>(), 100);
    EXPECT_FALSE(json.contains("time_used"));
}

TEST(VerdictBaseTest, ToJsonVerboseLevel1IncludesTiming) {
    VerdictBase verdict("test1");
    verdict.setInfo(
        {.type = VerdictType::WRONG_ANSWER, .score = 0, .time_used = 0.5, .memory_used = 512, .real_time_used = 0.6}); // NOLINT

    const auto json = verdict.toJson(1);

    EXPECT_DOUBLE_EQ(json["time_used"].get<double>(), 0.5);
    EXPECT_DOUBLE_EQ(json["memory_used"].get<double>(), 512);
    EXPECT_DOUBLE_EQ(json["real_time_used"].get<double>(), 0.6);
    EXPECT_FALSE(json.contains("additional_info"));
}

TEST(VerdictBaseTest, ToJsonVerboseLevel2IncludesAdditionalInfo) {
    VerdictBase verdict("test1");
    verdict.addToAdditionalInfo("subtest", nlohmann::json{{"score", 50}}); // NOLINT

    const auto json = verdict.toJson(2);

    EXPECT_TRUE(json.contains("additional_info"));
    EXPECT_EQ(json["additional_info"]["subtest"]["score"], 50);
}

TEST(VerdictBaseTest, ClearAdditionalInfoRemovesEntries) {
    VerdictBase verdict("test1");
    verdict.addToAdditionalInfo("subtest", nlohmann::json{{"score", 50}}); // NOLINT

    verdict.clearAdditionalInfo();

    EXPECT_TRUE(verdict.getAdditionalInfo().empty());
}

TEST(VerdictBaseTest, GetTestNameReturnsConstructorArgument) { EXPECT_EQ(VerdictBase("my_test").getTestName(), "my_test"); }
