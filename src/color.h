#ifndef COLOR_H
#define COLOR_H

#include <string>
#include <iostream>

class Color {
public:
    // ANSI escape codes for text color
    static const std::string RESET;
    static const std::string BLACK;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string WHITE;

    // Additional colors
    static const std::string BRIGHT_BLACK;
    static const std::string BRIGHT_RED;
    static const std::string BRIGHT_GREEN;
    static const std::string BRIGHT_YELLOW;
    static const std::string BRIGHT_BLUE;
    static const std::string BRIGHT_MAGENTA;
    static const std::string BRIGHT_CYAN;
    static const std::string BRIGHT_WHITE;

    // Rainbow colors
    static const std::string PINK;
    static const std::string PURPLE;
    static const std::string ORANGE;

    static void color_print(const std::string& message, const std::string& textColor);

    // Functions for individual color printing
    static void print_black(const std::string& message);
    static void print_red(const std::string& message);
    static void print_green(const std::string& message);
    static void print_yellow(const std::string& message);
    static void print_blue(const std::string& message);
    static void print_magenta(const std::string& message);
    static void print_cyan(const std::string& message);
    static void print_white(const std::string& message);

    // Additional color printing functions
    static void print_bright_black(const std::string& message);
    static void print_bright_red(const std::string& message);
    static void print_bright_green(const std::string& message);
    static void print_bright_yellow(const std::string& message);
    static void print_bright_blue(const std::string& message);
    static void print_bright_magenta(const std::string& message);
    static void print_bright_cyan(const std::string& message);
    static void print_bright_white(const std::string& message);

    // Rainbow color printing functions
    static void print_pink(const std::string& message);
    static void print_purple(const std::string& message);
    static void print_orange(const std::string& message);

    static void print_help();
    static void print_large_ascii_smiley();
};

#endif // COLOR_H
