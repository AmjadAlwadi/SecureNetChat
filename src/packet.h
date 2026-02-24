#ifndef PACKET_H
#define PACKET_H


#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>  

#include "command.h"
#include "validity.h"
#include "my_ssl.h"

class Packet {

public:
  
    Packet(const std::string& source,const std::string& destination, command_enum command, const std::string& data, EVP_PKEY* pky)
        : src(source), dest(destination),command(command), payload(data), pubkey(pky){}
    

    Packet(){};

    // Getter functions for instance variables
    std::string get_src() const;
    std::string get_dest() const;
    command_enum get_command() const;
    std::string get_payload() const;
    std::string command_tostring() const ;
    EVP_PKEY* get_pubkey() const;

    // Function to convert the packet to a string
    std::string to_string() const;

    std::string print();

    static Packet str_to_packet(const std::string& packet_str);
    std::string pubk_toString() const;
    static EVP_PKEY* pubk_fromString(const std::string& pubkey_str);
    static std::string format_key(const std::string& key);

    static void encrypt_packet(Packet& packet);
    
    

    std::string src;
    std::string dest;
    std::string payload;
    command_enum command;

    // the client must always add the their public key but the packet will not alway be encrypted
    EVP_PKEY* pubkey;  

};


#endif
