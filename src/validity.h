#ifndef VALIDITY_H
#define VALIDITY_H
#define MAX_PACKET_SIZE 1024 * 4
#define MAX_DELIMENTER 4
#define DELIMETER '\x04'
#define MAX_BUFF 1024 * 1024
#define MIN_COMMAND 0
#define MAX_COMMAND 10
#include <string>
#include <algorithm> 

class Validity {
public:
    static bool is_valid_input(const std::string& input);
    static bool is_valid_command(const std::string& input);
    static bool command_has_correct_parameter(const std::string& input, const std::string& command);
    static bool is_valid_packet_bytes(const char* bytes, size_t size);
    static bool delimiters(const char* bytes, size_t size);
    static bool is_valid_nickname(std::string ctx);
    static bool is_valid_string(const std::string& str);
    static bool unpack_bytes(const char* bytes, size_t size, std::string& src, std::string& dest, int& command, std::string& payload);
};

#endif // VALIDITY_H
