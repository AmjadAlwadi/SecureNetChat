#include "validity.h"
#include <sstream>
#include <cctype>
#include "color.h"

bool Validity::is_valid_input(const std::string& input) {
    if (input.size() >= MAX_BUFF) {
        return false;
    } 
    if (input.empty()) {
        return false;
    }
    if (!is_valid_string(input)){
        return false;
    } 
    return is_valid_command(input);
}
bool Validity::is_valid_command(const std::string& input) {
    std::string command_str;
    std::istringstream iss(input);
    iss >> std::ws >> command_str;

    std::transform(command_str.begin(), command_str.end(), command_str.begin(), ::toupper);

    if (command_str == "/LS" ||
        command_str == "/SHOW" ||
        command_str == "/HELP" ||
        command_str == "/NICK" ||
        command_str == "/MSG" ||
        command_str == "/JOIN" ||
        command_str == "/GETTOPIC" ||
        command_str == "/SETTOPIC" ||
        command_str == "/QUIT" ||
        command_str == "/LEAVE" ||
        command_str == "/LIST" ||
        command_str == "/GETMEMBERS" ||
        command_str == "/NEIGHBORS" ||
        command_str == "/PING" ||
        command_str == "/ROUTE" ||
        command_str == "/PLOT" ||
        command_str == "/GETPUBLICKEY" ||
        command_str == "/GETKEYPAIR") {
        return command_has_correct_parameter(input, command_str);
    } else {
        Color::print_bright_red("Unknown command");
        return false;
    }
}

bool Validity::command_has_correct_parameter(const std::string& input, const std::string& command) {
    // Ensure that the comparison strings are in uppercase
    std::string upper_command = command;
    std::transform(upper_command.begin(), upper_command.end(), upper_command.begin(), ::toupper);

    if (upper_command == "/NICK" ||
        upper_command == "/LEAVE" ||
        upper_command == "/GETTOPIC" ||
        upper_command == "/GETMEMBERS" ||
        upper_command == "/PING" ||
        upper_command == "/ROUTE" ||
        upper_command == "/GETPUBLICKEY") {
        std::string command_str, param_1;
        std::istringstream iss(input);
        iss >> std::ws >> command_str >> std::ws >> param_1;
        return !param_1.empty();
    }

    if (upper_command == "/SETTOPIC" ||
        upper_command == "/MSG" ||
        upper_command == "/JOIN") {
        std::string command_str, param_1, param_2;
        std::istringstream iss(input);
        iss >> std::ws >> command_str >> std::ws >> param_1 >> std::ws >> param_2;
        return !(param_1.empty() || param_2.empty());
    }

    if (upper_command == "/LS" ||
        upper_command == "/SHOW" ||
        upper_command == "/QUIT" ||
        upper_command == "/LIST" ||
        upper_command == "/HELP" ||
        upper_command == "/NEIGHBORS" ||
        upper_command == "/PLOT" ||
        upper_command == "/GETKEYPAIR") {
        return true;
    }

    return false;
}


bool Validity::is_valid_packet_bytes(const char* bytes, size_t size) {
    // Check the minimum size required for a valid packet

    if (!delimiters(bytes, size)) {
        Color::print_bright_red("Invalid packet: delimeters");
        return false;
    }

    std::string src, dest, payload;
    int command;

    if (!unpack_bytes(bytes, size, src, dest, command, payload)) {
        Color::print_bright_red("Invalid packet: unable to unpack");
        return false;
    }

    // Additional checks based on your packet structure
    // For example, ensure that the byte stream doesn't contain unexpected characters,
    // and validate the structure according to your protocol

    // Validate src, dest, and payload for alphanumeric with punctuation
    if (!is_valid_string(src) || !is_valid_string(dest) || !is_valid_string(payload)) {
        Color::print_bright_red("Invalid packet: unexpected characters in src, dest, or payload");
        return false;
    }

    // Validate command (assuming it's an integer in a specific range)
    if (command < MIN_COMMAND || command > MAX_COMMAND) {
        Color::print_bright_red("Invalid packet: command out of range");
        return false;
    }    
    return true;
}

bool Validity::delimiters(const char* bytes, size_t size) {


    int delimiter_count = 0;

    for (size_t i = 0; i < size; ++i) {
        if (bytes[i] == DELIMETER) {
            ++delimiter_count;
            
        }
    }
    return (delimiter_count == MAX_DELIMENTER);

}

bool Validity::is_valid_nickname(std::string nick){
    if (nick.length() > 9) {
        return false;
    }
    for (char c : nick) {
        if (!std::isalnum(c)) {
            return false;
        }
    }
    return true;
}

bool Validity::unpack_bytes(const char* bytes, size_t size, std::string& src, std::string& dest, int& command, std::string& payload) {
    std::istringstream iss(std::string(bytes, size));

    // Unpack the components based on your protocol
    std::string command_str;


    if (std::getline(iss, command_str, DELIMETER) &&
        std::getline(iss, src, DELIMETER) &&
        std::getline(iss, dest, DELIMETER) &&
        std::getline(iss, payload)) {

        // Convert the command string to an integer
        try {
            if (!command_str.empty()) {
                command = std::stoi(command_str);
            } else {
                // Handle the case where command_str is empty
                Color::print_bright_red("Command string is empty");
                return false;
            }
        } catch (const std::invalid_argument& e) {
            Color::print_bright_red("packt contains unknown command");
            return false;
        }
    }

    return true;
}

bool Validity::is_valid_string(const std::string& str) {
    // Validate that the string is alphanumeric with punctuation
    for (char c : str) {
        if (!std::isalnum(c) && !std::ispunct(c) && !std::isspace(c)) {
            Color::print_bright_red("The input must be alphanumeric, a punctuation character, or a space");
            return false;
        }

        if (c == '\'' || c == '"' || c == '\\') {
            Color::print_bright_red("Invalid input: contains \\ or \' or \" ");
            return false;
        }
    }
    return true;
}