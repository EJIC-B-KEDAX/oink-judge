#pragma once

#include <sqlite3.h>
#include <string>

namespace oink_judge::database {

class Statement {
public:
    Statement();
    ~Statement();

    void bind_text(const std::string &value, int place);
    void bind_int(int value, int place);

    sqlite3_stmt **get_stmt();

    int step();
    std::string column_text(int column) const;

private:
    sqlite3_stmt *_stmt;
};

} // namespace oink_judge::database
