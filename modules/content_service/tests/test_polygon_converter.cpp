#include "oink_judge/content_service/problem_package_converter/polygon_converter.h"

#include <gtest/gtest.h>
#include <pugixml.hpp>

#include <filesystem>
#include <string>
#include <vector>

using namespace oink_judge::content_service::problem_package_converter;
namespace fs = std::filesystem;

namespace {

auto findTestNodeByType(const pugi::xml_node& tests_node, const std::string& type) -> pugi::xml_node {
    for (pugi::xml_node test : tests_node.children("test")) {
        if (test.attribute("type").as_string() == type) {
            return test;
        }
    }
    return {};
}

auto collectTestNamesByType(const pugi::xml_node& tests_node, const std::string& type) -> std::vector<std::string> {
    std::vector<std::string> names;
    for (pugi::xml_node test : tests_node.children("test")) {
        if (test.attribute("type").as_string() == type) {
            names.emplace_back(test.attribute("name").as_string());
        }
    }
    return names;
}

} // namespace

class PolygonConverterTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = fs::path("resources") / "test_polygon_converter";
        work_ = resources_ / "_work";
        fs::remove_all(work_);
        fs::create_directories(work_);
    }

    auto TearDown() -> void override { fs::remove_all(work_); }

    // The conversion rewrites problem.xml in place and writes a checker binary,
    // so every test works on a disposable copy of the fixture package.
    auto preparePackage(const std::string& fixture_name) -> fs::path {
        fs::path package_path = work_ / fixture_name;
        fs::create_directories(package_path);
        fs::copy(resources_ / fixture_name, package_path, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        return package_path;
    }

    auto getWorkPath() -> const fs::path& { return work_; }

  private:
    fs::path resources_;
    fs::path work_;
};

// ---------------------------------------------------------------------------
// PackageConverterFactory
// ---------------------------------------------------------------------------

TEST_F(PolygonConverterTest, FactoryThrowsForUnknownType) {
    EXPECT_THROW((void)PackageConverterFactory::instance().create("unknown_type"), std::runtime_error);
}

TEST_F(PolygonConverterTest, PolygonConverterIsRegisteredUnderPolygonName) {
    std::shared_ptr<PackageConverter> converter;
    ASSERT_NO_THROW(converter = PackageConverterFactory::instance().create("polygon"));
    ASSERT_NE(converter, nullptr);
    EXPECT_NE(dynamic_cast<PolygonConverter*>(converter.get()), nullptr);
}

// ---------------------------------------------------------------------------
// convertPackage — error paths
// ---------------------------------------------------------------------------

TEST_F(PolygonConverterTest, MissingProblemXmlThrows) {
    fs::path empty_package = getWorkPath() / "empty_package";
    fs::create_directories(empty_package);

    PolygonConverter converter;
    EXPECT_THROW(converter.convertPackage(empty_package), std::runtime_error);
}

TEST_F(PolygonConverterTest, ProblemXmlWithoutProblemRootThrows) {
    fs::path package_path = preparePackage("bad_root");

    PolygonConverter converter;
    EXPECT_THROW(converter.convertPackage(package_path), std::runtime_error);
}

// ---------------------------------------------------------------------------
// convertPackage — happy path
// ---------------------------------------------------------------------------

TEST_F(PolygonConverterTest, HappyPathProducesExpectedProblemXml) {
    fs::path package_path = preparePackage("good_package");

    PolygonConverter converter;
    ASSERT_NO_THROW(converter.convertPackage(package_path));

    // Checker was compiled into the package directory
    EXPECT_TRUE(fs::exists(package_path / "checker"));

    pugi::xml_document doc;
    ASSERT_TRUE(doc.load_file((package_path / "problem.xml").c_str()));
    pugi::xml_node problem = doc.child("problem");
    ASSERT_TRUE(problem);

    EXPECT_STREQ(problem.attribute("type").as_string(), "open_problem");
    EXPECT_STREQ(problem.child("problem_builder").attribute("type").as_string(), "DefaultProblemBuilder");
    EXPECT_STREQ(problem.child("submission_manager").attribute("type").as_string(), "BasicProblemSubmissionManager");

    pugi::xml_node tests_node = problem.child("tests");
    ASSERT_TRUE(tests_node);

    // Per-test single entries: the "tests" testset name is special-cased,
    // so test names are "1", "2" rather than "tests1", "tests2".
    std::vector<std::string> single_names = collectTestNamesByType(tests_node, "single");
    EXPECT_EQ(single_names, (std::vector<std::string>{"1", "2"}));

    // Testset node with limits; real_time_limit = max(2 * 1000, 5000) = 5000
    pugi::xml_node testset = findTestNodeByType(tests_node, "testset");
    ASSERT_TRUE(testset);
    EXPECT_STREQ(testset.attribute("name").as_string(), "tests");
    EXPECT_EQ(testset.child("time_limit").text().as_ullong(), 1000ULL);
    EXPECT_EQ(testset.child("memory_limit").text().as_ullong(), 268435456ULL);
    EXPECT_EQ(testset.child("real_time_limit").text().as_ullong(), 5000ULL);
    EXPECT_STREQ(testset.child("verdict_builder").attribute("type").as_string(), "min");

    std::vector<std::string> testset_children;
    for (pugi::xml_node test : testset.children("test")) {
        testset_children.emplace_back(test.attribute("name").as_string());
    }
    EXPECT_EQ(testset_children, (std::vector<std::string>{"1", "2"}));

    // Compilation and sync_result service nodes
    pugi::xml_node compilation = findTestNodeByType(tests_node, "compilation");
    ASSERT_TRUE(compilation);
    EXPECT_STREQ(compilation.attribute("name").as_string(), "compilation");
    EXPECT_STREQ(compilation.child("test").attribute("name").as_string(), "tests");

    pugi::xml_node sync_result = findTestNodeByType(tests_node, "sync_result");
    ASSERT_TRUE(sync_result);
    EXPECT_STREQ(sync_result.attribute("name").as_string(), "main");
    EXPECT_STREQ(sync_result.child("test").attribute("name").as_string(), "compilation");
}
