#include "oink_judge/config/config.h"
#include "oink_judge/config/problem_config_utils.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <string>

using namespace oink_judge::config;
using namespace oink_judge::problem_config;
namespace fs = std::filesystem;

class ProblemConfigTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = fs::path("resources") / "test_problem_config";
        ProblemConfigTest::loadConfig(resources_ / "good_config.json", resources_ / "good_credentials.json");
    }

    auto getResourcesPath() -> const fs::path& { return resources_; }

    static auto loadConfig(const fs::path& config, const fs::path& credentials) -> void {
        Config::setConfigFilePath(config);
        Config::setCredentialsFilePath(credentials);
        Config::reloadData();
    }

  private:
    fs::path resources_;
};

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsCorrectTestNames) {
    auto all_tests = getAllTestNames("problem_two_tests");
    ASSERT_TRUE(all_tests.has_value());
    ASSERT_EQ(all_tests->size(), 2);
    EXPECT_EQ((*all_tests)[0], "t1");
    EXPECT_EQ((*all_tests)[1], "t2");
}

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsEmptyForProblemWithNoTests) {
    auto empty_tests = getAllTestNames("problem_no_tests");
    ASSERT_TRUE(empty_tests.has_value());
    EXPECT_TRUE(empty_tests->empty());
}

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsNulloptForNoTestsNode) {
    EXPECT_FALSE(getAllTestNames("problem_no_node_tests").has_value());
}

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsNulloptForMissingProblem) {
    EXPECT_FALSE(getAllTestNames("non_existent_problem").has_value());
}

TEST_F(ProblemConfigTest, GetProblemBuilderNameReturnsCorrectName) {
    auto builder = getProblemBuilderName("problem_two_tests");
    ASSERT_TRUE(builder.has_value());
    EXPECT_EQ(*builder, "default");
}

TEST_F(ProblemConfigTest, GetProblemBuilderNameReturnsNulloptForMissingBuilder) {
    EXPECT_FALSE(getProblemBuilderName("problem_no_builder").has_value());
}

TEST_F(ProblemConfigTest, GetTestConfigReturnsCorrectNode) {
    auto tnode = getTestConfig("problem_two_tests", "t2");
    ASSERT_TRUE(tnode.has_value());
    EXPECT_EQ(std::string(tnode->attribute("name").as_string()), "t2");
}

TEST_F(ProblemConfigTest, GetTestConfigReturnsNulloptForMissingTest) {
    EXPECT_FALSE(getTestConfig("problem_two_tests", "non_existent_test").has_value());
}

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsNulloptWhenProblemsDirMissing) {
    loadConfig(getResourcesPath() / "bad_no_problems.json", getResourcesPath() / "good_credentials.json");
    EXPECT_FALSE(getAllTestNames("problem_two_tests").has_value());
}

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsNulloptForMalformedXml) {
    EXPECT_FALSE(getAllTestNames("problem_bad_xml").has_value());
}

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsNulloptWhenConfigMissing) {
    EXPECT_FALSE(getAllTestNames("problem_no_config").has_value());
}

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsNulloptWhenConfigEmpty) {
    EXPECT_FALSE(getAllTestNames("problem_empty_config").has_value());
}

TEST_F(ProblemConfigTest, GetAllTestNamesReturnsNulloptForUnexistentProblem) {
    EXPECT_FALSE(getAllTestNames("unexistent_problem").has_value());
}
