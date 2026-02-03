#include <iostream>

int main() {
    // Regular colors
    std::cout << "Regular colors:\n";
    std::cout << "\033[30m Black \033[0m\n";
    std::cout << "\033[31m Red \033[0m\n";
    std::cout << "\033[32m Green \033[0m\n";
    std::cout << "\033[33m Yellow \033[0m\n";
    std::cout << "\033[34m Blue \033[0m\n";
    std::cout << "\033[35m Magenta \033[0m\n";
    std::cout << "\033[36m Cyan \033[0m\n";
    std::cout << "\033[37m White \033[0m\n";

    // Bright colors
    std::cout << "\nBright colors:\n";
    std::cout << "\033[90m Bright Black \033[0m\n";
    std::cout << "\033[91m Bright Red \033[0m\n";
    std::cout << "\033[92m Bright Green \033[0m\n";
    std::cout << "\033[93m Bright Yellow \033[0m\n";
    std::cout << "\033[94m Bright Blue \033[0m\n";
    std::cout << "\033[95m Bright Magenta \033[0m\n";
    std::cout << "\033[96m Bright Cyan \033[0m\n";
    std::cout << "\033[97m Bright White \033[0m\n";

    // Background colors
    std::cout << "\nBackground colors:\n";
    std::cout << "\033[40m Black BG \033[0m\n";
    std::cout << "\033[41m Red BG \033[0m\n";
    std::cout << "\033[42m Green BG \033[0m\n";
    std::cout << "\033[43m Yellow BG \033[0m\n";
    std::cout << "\033[44m Blue BG \033[0m\n";
    std::cout << "\033[45m Magenta BG \033[0m\n";
    std::cout << "\033[46m Cyan BG \033[0m\n";
    std::cout << "\033[47m White BG \033[0m\n";

    // Dark red (using dim style with red)
    std::cout << "\nDark red:\n";
    std::cout << "\033[2;31m Dark Red (Dim + Red) \033[0m\n";
    std::cout << "\033[38;5;88m Dark Red (256-color) \033[0m\n";
    std::cout << "\033[38;2;139;0;0m Dark Red (RGB) \033[0m\n";

    // Styles
    std::cout << "\nText styles:\n";
    std::cout << "\033[1m Bold \033[0m\n";
    std::cout << "\033[2m Dim \033[0m\n";
    std::cout << "\033[3m Italic \033[0m\n";
    std::cout << "\033[4m Underline \033[0m\n";
    std::cout << "\033[7m Reverse \033[0m\n";

    return 0;
}
