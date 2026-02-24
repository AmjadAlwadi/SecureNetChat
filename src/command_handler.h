#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string.h>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include "color.h"
#include "validity.h"
#include "connected_client.h"
#include "my_ssl.h"

class Client;

class Command_Handler {
public:

    Client* me;

    Command_Handler() = default;
    
    bool join(std::string ctx);

    bool leave(std::string ctx);

    bool get_topic(std::string ctx);

    bool set_topic(std::string ctx);

    bool msg(std::string ctx);

    bool quit();

    bool list();

    bool handle_command(std::string ctx);

    bool nick(std::string ctx);

    bool get_members(std::string ctx);

    bool neighbors();

    bool ping(std::string ctx);

    bool route(std::string ctx);
    
    bool plot();

    bool show_clients_list();
    
    bool get_help();

    bool ls();

    bool get_key_pair();

    bool get_public_key(std::string ctx);

    bool find_client(std::string name, struct Connected_Client&client);

    bool find_group(std::string name, struct Group&group);
    
    bool share_secret(Group& g, const std::string& secret);
    
    std::string find_secret_owner(const struct Group& g);
    

   
};

#endif