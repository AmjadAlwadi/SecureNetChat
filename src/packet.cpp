#include "packet.h"

std::string Packet::to_string() const {
        return std::to_string(static_cast<int>(command)) +
            "\x04" + src +
            "\x04" + dest +
            "\x04" + payload +
            "\x04" + pubk_toString();
    
}

std::string Packet::print() {
    return "[Command : " + std::to_string(static_cast<int>(command)) +
           "| src : " + src +
           " ---> dest : " + dest +
           "| Payload : " + payload +
           "| Public Key : " + pubk_toString() + "]";
}

Packet Packet::str_to_packet(const std::string& packet_str) {
    std::istringstream iss(packet_str);
    std::string command_str, src, dest, payload, pubkey_str;

    // Read each field from the input string
    std::getline(iss, command_str, '\x04');
    std::getline(iss, src, '\x04');
    std::getline(iss, dest, '\x04');
    std::getline(iss, payload, '\x04');
    
    std::getline(iss, pubkey_str,  '\x04');

    std::string pky_str = format_key(pubkey_str);

    command_enum command = command_enum::NACK;

    try {
        command = static_cast<command_enum>(std::stoi(command_str));
    } catch (std::exception& e) {
       
        std::cerr << "Error converting command: " << e.what() << std::endl;
    }

    EVP_PKEY* pky = pubk_fromString(pky_str);
    if (pky) {
        return Packet(src, dest, command, payload, pky);
    } else {
        std::cerr << "No Public key." << std::endl;
        Packet packet;
        packet.src = src;
        packet.dest = dest;
        packet.payload = payload;
        return packet;
    }
}


std::string Packet::get_src() const { return src; }
std::string Packet::get_dest() const { return dest; }
command_enum Packet::get_command() const { return command; }
std::string Packet::get_payload() const { return payload; }
EVP_PKEY* Packet::get_pubkey() const {return pubkey;}

std::string Packet::pubk_toString() const {
    return my_SSL::pubk_toString(pubkey);
}

EVP_PKEY* Packet::pubk_fromString(const std::string& pubkey_str) {
   return my_SSL::pubk_fromString(pubkey_str);
}
std::string Packet:: format_key(const std::string& key){
   
    return my_SSL::format_key(key);
}

void Packet::encrypt_packet(Packet&packet) {
    std::string cipher = my_SSL::encrypt(packet.payload, packet.pubkey);
    packet.payload = cipher;
}
