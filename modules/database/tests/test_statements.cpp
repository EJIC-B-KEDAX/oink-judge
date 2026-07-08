#include <oink_judge/database/statements.h>

#include <gtest/gtest.h>

using oink_judge::database::Statement;
using oink_judge::database::StatementsBlock;

TEST(StatementTest, StoresNameAndSql) {
    const Statement statement("users__select", "SELECT 1");
    EXPECT_EQ(statement.name(), "users__select");
    EXPECT_EQ(statement.sql(), "SELECT 1");
}

TEST(StatementsBlockTest, AddsAndRetrievesStatements) {
    StatementsBlock block("users");
    block.addStatement(Statement("users__select", "SELECT 1"));
    block.addStatement(Statement("users__insert", "INSERT INTO users VALUES ($1)"));

    EXPECT_EQ(block.name(), "users");
    ASSERT_EQ(block.statements().size(), 2U);
    EXPECT_EQ(block.getStatement("users__select").sql(), "SELECT 1");
    EXPECT_EQ(block.getStatement("users__insert").sql(), "INSERT INTO users VALUES ($1)");
}

TEST(StatementsBlockTest, OverwritesStatementWithSameName) {
    StatementsBlock block("users");
    block.addStatement(Statement("users__select", "SELECT 1"));
    block.addStatement(Statement("users__select", "SELECT 2"));
    EXPECT_EQ(block.getStatement("users__select").sql(), "SELECT 2");
}

TEST(StatementsBlockTest, ThrowsForMissingStatement) {
    StatementsBlock block("users");
    EXPECT_THROW((void)block.getStatement("missing"), std::out_of_range);
}
