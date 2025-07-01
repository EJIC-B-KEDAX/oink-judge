#include <iostream>
#include <sqlite3.h>

int32_t main() {
    std::cout << sqlite3_libversion() << std::endl;
}
