#pragma once
#include <string>
#include <unordered_map>

namespace oink_judge::database {

class Statement {
  public:
    Statement(std::string name, std::string sql);

    [[nodiscard]] auto name() const -> const std::string&;
    [[nodiscard]] auto sql() const -> const std::string&;

  private:
    std::string name_;
    std::string sql_;
};

class StatementsBlock {
  public:
    explicit StatementsBlock(std::string block_name);

    [[nodiscard]] auto name() const -> const std::string&;

    auto addStatement(Statement statement) -> void;
    [[nodiscard]] auto getStatement(const std::string& name) const -> const Statement&;

    [[nodiscard]] auto statements() const -> const std::unordered_map<std::string, Statement>&;

  private:
    std::string block_name_;
    std::unordered_map<std::string, Statement> statements_;
};

} // namespace oink_judge::database
