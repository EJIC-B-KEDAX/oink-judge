#include <oink_judge/database/statement_action_log.h>
#include <oink_judge/database/statements.h>

#include <gtest/gtest.h>

using oink_judge::database::Statement;
using oink_judge::database::StatementActionLog;
using oink_judge::database::StatementActionType;
using oink_judge::database::StatementsBlock;

namespace {

auto makeUsersBlock() -> StatementsBlock {
    StatementsBlock block("users");
    block.addStatement(Statement("users__select", "SELECT 1"));
    return block;
}

} // namespace

TEST(StatementActionLogTest, AppendsPrepareAndUnprepare) {
    StatementActionLog log;
    EXPECT_EQ(log.appendPrepare(makeUsersBlock()), 1);
    EXPECT_EQ(log.appendUnprepare("users"), 2);

    const auto& actions = log.actions();
    ASSERT_EQ(actions.size(), 2);
    EXPECT_EQ(actions[0].type, StatementActionType::PREPARE);
    EXPECT_EQ(actions[0].block.name(), "users");
    EXPECT_EQ(actions[1].type, StatementActionType::UNPREPARE);
    EXPECT_EQ(actions[1].block.name(), "users");
}

TEST(StatementActionLogTest, RejectsDuplicatePrepare) {
    StatementActionLog log;
    log.appendPrepare(makeUsersBlock());
    EXPECT_THROW(log.appendPrepare(makeUsersBlock()), std::runtime_error);
}

TEST(StatementActionLogTest, RejectsUnprepareForUnknownBlock) {
    StatementActionLog log;
    EXPECT_THROW(log.appendUnprepare("missing"), std::runtime_error);
}

TEST(StatementActionLogTest, PreservesPreparedStatementsInBlock) {
    StatementActionLog log;
    log.appendPrepare(makeUsersBlock());

    const auto& actions = log.actions();
    ASSERT_EQ(actions[0].block.statements().size(), 1U);
    EXPECT_EQ(actions[0].block.getStatement("users__select").sql(), "SELECT 1");
}
