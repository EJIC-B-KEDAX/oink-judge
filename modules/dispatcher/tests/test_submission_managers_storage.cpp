#include "oink_judge/dispatcher/send_submission_to_invoker.h"
#include "oink_judge/dispatcher/submission_managers_storage.h"

#include <gtest/gtest.h>

using namespace oink_judge::dispatcher;

class SubmissionManagersStorageTest : public ::testing::Test {
  protected:
    static auto SetUpTestSuite() -> void { registerSendSubmissionToInvokerType(); }
};

TEST_F(SubmissionManagersStorageTest, ReturnsSameManagerForSameProblem) {
    auto& storage = SubmissionManagersStorage::instance();

    auto first = storage.getSubmissionManager("problem-a");
    auto second = storage.getSubmissionManager("problem-a");

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

TEST_F(SubmissionManagersStorageTest, ReturnsDifferentManagersForDifferentProblems) {
    auto& storage = SubmissionManagersStorage::instance();

    auto manager_a = storage.getSubmissionManager("problem-b");
    auto manager_b = storage.getSubmissionManager("problem-c");

    ASSERT_NE(manager_a, nullptr);
    ASSERT_NE(manager_b, nullptr);
    EXPECT_NE(manager_a, manager_b);
}
