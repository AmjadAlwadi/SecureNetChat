#include "color.h"

const std::string Color::RESET = "\033[0m";
const std::string Color::BLACK = "\033[30m";
const std::string Color::RED = "\033[31m";
const std::string Color::GREEN = "\033[32m";
const std::string Color::YELLOW = "\033[33m";
const std::string Color::BLUE = "\033[34m";
const std::string Color::MAGENTA = "\033[35m";
const std::string Color::CYAN = "\033[36m";
const std::string Color::WHITE = "\033[37m";

const std::string Color::BRIGHT_BLACK = "\033[90m";
const std::string Color::BRIGHT_RED = "\033[91m";
const std::string Color::BRIGHT_GREEN = "\033[92m";
const std::string Color::BRIGHT_YELLOW = "\033[93m";
const std::string Color::BRIGHT_BLUE = "\033[94m";
const std::string Color::BRIGHT_MAGENTA = "\033[95m";
const std::string Color::BRIGHT_CYAN = "\033[96m";
const std::string Color::BRIGHT_WHITE = "\033[97m";

const std::string Color::PINK = "\033[38;5;206m";
const std::string Color::PURPLE = "\033[38;5;129m";
const std::string Color::ORANGE = "\033[38;5;214m";

void Color::color_print(const std::string& message, const std::string& textColor) {
    // ANSI escape codes for terminal colors
    std::cout << textColor << message << RESET << std::endl;
}

void Color::print_black(const std::string& message) {
    color_print(message, BLACK);
}

void Color::print_red(const std::string& message) {
    color_print(message, RED);
}

void Color::print_green(const std::string& message) {
    color_print(message, GREEN);
}

void Color::print_yellow(const std::string& message) {
    color_print(message, YELLOW);
}

void Color::print_blue(const std::string& message) {
    color_print(message, BLUE);
}

void Color::print_magenta(const std::string& message) {
    color_print(message, MAGENTA);
}

void Color::print_cyan(const std::string& message) {
    color_print(message, CYAN);
}

void Color::print_white(const std::string& message) {
    color_print(message, WHITE);
}

void Color::print_bright_black(const std::string& message) {
    color_print(message, BRIGHT_BLACK);
}

void Color::print_bright_red(const std::string& message) {
    color_print(message, BRIGHT_RED);
}

void Color::print_bright_green(const std::string& message) {
    color_print(message, BRIGHT_GREEN);
}

void Color::print_bright_yellow(const std::string& message) {
    color_print(message, BRIGHT_YELLOW);
}

void Color::print_bright_blue(const std::string& message) {
    color_print(message, BRIGHT_BLUE);
}

void Color::print_bright_magenta(const std::string& message) {
    color_print(message, BRIGHT_MAGENTA);
}

void Color::print_bright_cyan(const std::string& message) {
    color_print(message, BRIGHT_CYAN);
}

void Color::print_bright_white(const std::string& message) {
    color_print(message, BRIGHT_WHITE);
}

void Color::print_pink(const std::string& message) {
    color_print(message, PINK);
}

void Color::print_purple(const std::string& message) {
    color_print(message, PURPLE);
}

void Color::print_orange(const std::string& message) {
    color_print(message, ORANGE);
}

void Color::print_help() {
   
    Color::print_bright_white("Available commands:\n");

    Color::print_yellow("JOIN <name> ");
    std::cout << "\t- Join a group with the specified name.\n";

    Color::print_yellow("LEAVE <name> ");
    std::cout << "\t- Leave the group with the specified name.\n";

    Color::print_yellow("NICK <nickname> ");
    std::cout << "\t- Set your nickname to the specified value.\n\tNicknames can be up to nine characters long and must consist of letters or numbers.\n";

    Color::print_yellow("LIST ");
    std::cout << "\t- List all groups existing in IBRC with their short names.\n";

    Color::print_yellow("GETTOPIC <group> ");
    std::cout << "\t- Output the detailed description (topic) for the specified group. Displays an error message if the group does not exist.\n";

    Color::print_yellow("SETTOPIC <group> <topic> ");
    std::cout << "\t- Set the topic of the specified group.\n\tOnly the group creator can execute this command.\n\tOther users' attempts will be acknowledged with an error message.\n";

    Color::print_yellow("GETMEMBERS <group> ");
    std::cout << "\t- Outputs a list of all group members in the specified group.\n";

    Color::print_yellow("NEIGHBORS ");
    std::cout << "\t- Shows all direct neighbors.\n";

    Color::print_yellow("PING [<nickname>/IP] ");
    std::cout << "\t- Determines the availability and the Round-Trip-Time (RTT) to the specified node, identified by its nickname or IP.\n";

    Color::print_yellow("ROUTE [<name>] ");
    std::cout << "\t- Shows the route to the destination. With the optional specification <name>,\n\tit should be recognizable through which intermediate stations (hops) a message reaches its destination. Without the specification of <name>, the current routing table of the client shall be displayed. This table shows which clients can be reached over which port.\n";

    Color::print_yellow("PLOT ");
    std::cout << "\t- Creates a graphical representation/view of the current network structure, similar to Figure 2. An external tool, like graphviz3, is recommended for this purpose.\n";

    Color::print_yellow("MSG <nickname> <message> ");
    std::cout << "\t- Sends a message to the user or group specified in <nickname>. If the specified user or group does not exist, an error message is displayed. The message doesn’t require quotation marks for multiple words. Everything behind the name is treated as one argument.\n";

    Color::print_yellow("QUIT ");
    std::cout << "\t- Ends the client. If you still belong to groups, membership in groups is automatically terminated.\n";

}

void Color::print_large_ascii_smiley() {
    print_bright_cyan("\t\t\t\tWelcome to the IBRC Chat Client!");
    print_bright_cyan("\t\t\t\tType /HELP to see available commands.");

    std::cout << "\n";
    std::cout << "   *****   \n";
    std::cout << " *       * \n";
    std::cout << "*  O   O  *\n";
    std::cout << "*    ∆    *\n";
    std::cout << " *  \\_/  * \n";
    std::cout << "   *****   \n";
    std::cout << "\n";
}