#include <oink_judge/database/statement_action_log.h>

#include <gtest/gtest.h>

using oink_judge::database::StatementActionLog;
using oink_judge::database::StatementActionType;

TEST(StatementActionLogTest, AppendsPrepareAndUnprepare) {
    StatementActionLog log;
    EXPECT_EQ(log.appendPrepare("users__select", "SELECT 1"), 1);
    EXPECT_EQ(log.appendUnprepare("users__select"), 2);

    const auto& actions = log.actions();
    ASSERT_EQ(actions.size(), 2);
    EXPECT_EQ(actions[0].type, StatementActionType::PREPARE);
    EXPECT_EQ(actions[0].name, "users__select");
    EXPECT_EQ(actions[1].type, StatementActionType::UNPREPARE);
}

TEST(StatementActionLogTest, RejectsDuplicatePrepare) {
    StatementActionLog log;
    log.appendPrepare("stmt", "SELECT 1");
    EXPECT_THROW(log.appendPrepare("stmt", "SELECT 2"), std::runtime_error);
}
