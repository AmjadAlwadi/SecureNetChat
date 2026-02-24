#include "command_handler.h"
#include "client.h"

bool Command_Handler::join(std::string ctx) {
    // Extract group name from the context
    std::istringstream iss(ctx);
    std::string cmd_type, group_name, secret;
    iss >> cmd_type >> group_name; // Extracting "JOIN" and the group name
    std::getline(iss, secret);


    // Check if exists

    for(struct Group& item : me->groups){

        if(group_name.compare(item.name) == 0){

            for(auto name : item.members_nicknames){
                if(name == me->nickname){
                    Color::print_orange("Already in this group");
                    return true;
                }
            }

            item.members_nicknames.push_back(me->nickname);

            me->distribute_group_information(item);
            share_secret(item, secret);
            Color::print_green("joined group " + group_name + " successfully");
            return true;

        }
    }



    // If not

    struct Group new_group;

    new_group.name = group_name;
    new_group.owner_nickname = me->nickname;
    new_group.members_nicknames.push_back(me->nickname);
    new_group.pkey = my_SSL::get_public_key(group_name);

    me->distribute_group_information(new_group);
    share_secret(new_group, secret);

    Color::print_green("created group " + group_name + " successfully");

    return true;  
}


bool Command_Handler::share_secret(Group& g, const std::string& secret) {
    bool sent = false; // Track if at least one secret is shared


    for(auto &nick_name : g.members_nicknames) {
        Connected_Client cc;
        if (find_client(nick_name, cc)) {
            // Sent
            Packet packet;
            packet.dest = cc.nickname;
            packet.src = g.name + "|" + me->nickname;
            packet.command = command_enum::FORWARD;

            std::vector<unsigned char> key; 
            if(my_SSL::gen_aes(secret, key)) { 
                std::string key_str(key.begin(), key.end()); 
                packet.payload = key_str;


                struct Group_Secret gs;
                gs.nickname = me->nickname;
                gs.key = key_str;

                g.secrets.push_back(gs);
                packet.pubkey = cc.pkey;
            
                me->forward_packet(packet);
                Color::print_green("shared secret" + nick_name + " in group " + g.name + " successfully");
                sent = true; // Mark as sent
               
            } else {
                Color::print_red("Failed to generate AES key for " + nick_name);
            }
        }
    }

    return sent; // Return true if at least one secret was shared
}



bool Command_Handler::leave(std::string ctx) {
    // Extract group name from the context
    std::istringstream iss(ctx);
    std::string cmd_type, group_name;
    iss >> cmd_type >> group_name; // Extracting "LEAVE" and the group name

    //check if exists and if it does then remove client from the group

    struct Group new_group;

    for(auto& group:me->groups){

        if(group.name == group_name){

            for(int i = 0;i<group.members_nicknames.size();i++){

                if(group.members_nicknames.at(i) == me->nickname){

                    group.members_nicknames.erase(group.members_nicknames.begin() + i);
                    new_group = group;
                }
            }
        }
    }

    Color::print_green("left group " + group_name + " successfully");

    me->distribute_group_information(new_group);

    return true;  
}


bool Command_Handler::get_topic(std::string ctx) {
    // Extract group name from the context
    std::istringstream iss(ctx);
    std::string cmd_type, group_name;
    iss >> cmd_type >> group_name; // Extracting "GETTOPIC" and the group name

    for(auto &item : me->groups){

        if(item.name == group_name){

            Color::print_green("Topic of " + group_name + " : " + item.topic);
            return true;
        }
    }

    Color::print_red("Couldn't get topic");
    return true; 
    
}



bool Command_Handler::set_topic(std::string ctx) {
    // Extract group name and topic from the context
    std::istringstream iss(ctx);
    std::string cmd_type, group_name, new_topic;
    iss >> cmd_type >> group_name >> std::ws;
    std::getline(iss, new_topic); // Extracting "SETTOPIC," group name, and the topic


    for(struct Group& item : me->groups){

        if(item.name == group_name && item.owner_nickname == me->nickname){

            item.topic = new_topic;
            me->distribute_group_information(item);
            Color::print_green("Set topic to " + item.topic + " successfully");
            return true;
        }

    }

    Color::print_red("Couldn't set topic");
    return true; 
}


bool Command_Handler::get_members(std::string ctx) {

    std::istringstream iss(ctx);
    std::string cmd_type, group_name;
    iss >> cmd_type >> group_name; // Extracting "GETMEMBERS" and the group name

    std::string groups_string = "";

    for(struct Group& item : me->groups){

        if(item.name == group_name){

           for(auto name : item.members_nicknames ){

                groups_string.append(name + " ");
               
            }
        }
    }

    Color::print_green(groups_string);

    return true; 
}



bool Command_Handler::neighbors() {

    std::string left_nickname = "";
    std::string right_nickname = "";

    for(auto item : me->clients_list){

        if(me->left == item.number){
            left_nickname = item.nickname;
        }

        if(me->right == item.number){
            right_nickname = item.nickname;
        }

    }

    Color::print_green(left_nickname + " <------ " + me->nickname + " ------> " + right_nickname);
    return true; 
}




bool Command_Handler::ping(std::string ctx) {

    // Ping the specified node nickname and measure Round-Trip-Time
    std::istringstream iss(ctx);
    std::string cmd_type, node;
    iss >> cmd_type >> node; // Extracting "PING" and the nickname or IP

    struct timeval tv;
    gettimeofday(&tv, nullptr); // Get the current time in microseconds


    long long milliseconds = static_cast<long long>(tv.tv_sec) * 1000LL + static_cast<long long>(tv.tv_usec) / 1000LL;

    struct Connected_Client cc;
    struct Group g;
    if (find_client(node, cc)) {
        Packet ping_packet;
        ping_packet.src = me->nickname;
        ping_packet.dest = node;
        ping_packet.command = command_enum::FORWARD;
        ping_packet.payload = std::string(1,BEGINPINGSYMBOL) + std::to_string(milliseconds);
        ping_packet.pubkey = cc.pkey;
        return me->forward_packet(ping_packet);
        
    }
    if (find_group(node, g)) {
        Packet ping_packet;
        ping_packet.src = me->nickname;
        ping_packet.dest = node;
        ping_packet.command = command_enum::FORWARD;
        ping_packet.payload = std::string(1,BEGINPINGSYMBOL) + std::to_string(milliseconds);
        ping_packet.pubkey = g.pkey;
        return me->forward_packet(ping_packet);
    }

    return false;
}



bool Command_Handler::route(std::string ctx) {
    // Display the route to the destination (specified by name)
    // Extract nickname from the context
    std::istringstream iss(ctx);
    std::string cmd_type, nickname;
    iss >> cmd_type >> nickname; // Extracting "LEAVE" and the group name


    // Find number of dest

    int number = -1;

    for(auto item : me->clients_list){

        if(item.nickname == nickname) number = item.number;

    }

    if(number == -1) {

        Color::print_red("That client was not found !");
        return true;
    }



    int distance = std::abs(me->my_number - number);

    // With or without rotation
    int distance_right_with = me->calculate_number() - 1 - me->my_number + (number + 1) ;
    int distance_right_without = distance;
    int distance_left_with = me->my_number + me->calculate_number() - 1 - number + 1 ;
    int distance_left_without = distance;

    std::string route = me->nickname;

    // Forward to the left client
    if(number < me->my_number){

        if(distance_left_without <= distance_right_with){
            
            for(int i = me->my_number - 1 ; i >= number ; i--){

                for(auto item : me->clients_list){
                    if(item.number == i) route.append(" ----> " + item.nickname);
                }

            }           
        }

        else{

            for(int i = me->my_number + 1 ; i <= me->calculate_number() - 1 ; i++){

                for(auto item : me->clients_list){
                    if(item.number == i) route.append(" ----> " + item.nickname);
                }

            }

            for(int i = 0 ; i <= number ; i++){

                for(auto item : me->clients_list){
                    if(item.number == i) route.append(" ----> " + item.nickname);
                }

            }
            
        }
    }


    // Forward to the right server
    else if(number > me->my_number){

        if(distance_right_without <= distance_left_with){

            for(int i = me->my_number + 1 ; i <= number ; i++){

                for(auto item : me->clients_list){
                    if(item.number == i) route.append(" ----> " + item.nickname);
                }
            }
        }

        else{

            for(int i = me->my_number - 1 ; i >= 0 ; i--){

                for(auto item : me->clients_list){
                    if(item.number == i) route.append(" ----> " + item.nickname);
                }

            }

            for(int i = me->calculate_number() - 1 ; i >= number ; i--){

                for(auto item : me->clients_list){
                    if(item.number == i) route.append(" ----> " + item.nickname);
                }
            }
        }
    }

    Color::print_green(route);
    return true; 
}


bool Command_Handler::show_clients_list(){

    for(auto item : me->clients_list){

        Color::print_purple(item.hostname + ", " + item.nickname + ", " + std::to_string(item.number));

    }

    return true;
}



bool Command_Handler::plot() {
    // Implementation for PLOT command
    // Create a graphical representation/view of the current network structure

    std::string dotString = "graph CircularNetworkTopology {\n";

    int numClients = me->clients_list.size();

    // Connect each client to its left and right neighbors
    for (int i = 0; i < numClients - 1; i++) {

        std::string current_nickname;

        for(auto item : me->clients_list){
            if(i == item.number ){
                current_nickname = item.nickname;
            }
        }

        

        int left_neighbor = (i == 0) ? numClients - 1 : i - 1; // Connect to the last client in a circular manner
        int right_neighbor = (i == numClients - 1) ? 0 : i + 1; // Connect to the root in a circular manner


        if(i == 0 && numClients > 2){  

            dotString += "  " + current_nickname + " -- " + me->find_nickname_of_number(left_neighbor) + ";\n";
            dotString += "  " + current_nickname + " -- " + me->find_nickname_of_number(right_neighbor) + ";\n";

        }else{
            dotString += "  " + current_nickname + " -- " + me->find_nickname_of_number(right_neighbor) + ";\n";
        }

        
    }

    dotString += "}\n";

    Color::print_purple(dotString);


    // Save DOT string to a file
    std::ofstream dotFile("network_topology.dot");
    dotFile << dotString;
    dotFile.close();

    // Generate PNG image using Graphviz 'dot' command
    std::string dotCommand = "dot -Tpng " + std::string("network_topology.dot") + " -o " + std::string("network_topology.png");
    int result = std::system(dotCommand.c_str());

    if (result == 0) {
        std::cout << "Network topology image saved as '" << std::string("network_topology.png") << "'.\n";
    } else {
        std::cerr << "Error generating the network topology image.\n";
    }


    return true;
}

std::string Command_Handler:: find_secret_owner(const struct Group& g) {
     for (const Group_Secret& gs : g.secrets) {
        if (gs.nickname == me->nickname) {
            
            Color::print_yellow(gs.key);
            return gs.key; // Return the key if the owner is found
        }
    }
}




bool Command_Handler::msg(std::string ctx) {
    std::istringstream iss(ctx);
    std::string cmd_type, dest, message;
    iss >> cmd_type >> dest;

    // Read the rest of the line as the message
    std::getline(iss, message);
    
    //Case for group

    struct Group g;

    bool found = find_group(dest, g);
   

    if (found){
        for(auto &nick_name : g.members_nicknames){

            if(nick_name == me->nickname) continue;

            struct Connected_Client cc;
            if (find_client(nick_name, cc)) {
                // Sent
                Packet msg_packet;
                msg_packet.dest = nick_name;
                msg_packet.src = me->nickname;
                msg_packet.command = command_enum::FORWARD;

                std::string key = find_secret_owner(g);
                if (key.empty()) {
                    Color::print_red("no secret found!");
                }
                
                msg_packet.payload =my_SSL::aes_encrypt(message, reinterpret_cast<const unsigned char*>(key.c_str()));
                msg_packet.pubkey = g.pkey;
               
                me->forward_packet(msg_packet);

                Color::print_green("Sent message to " + g.name + " successfully");
                return true; 
            }

        }
    }


    // Case for client

    for(auto& item : me->clients_list){

        if(item.nickname == dest){

            // ACK
            Color::print_green("Sent message to " + me->nickname + " successfully");


            // Sent
            Packet msg_packet;
            msg_packet.dest = dest;
            msg_packet.src = me->nickname;
            msg_packet.command = command_enum::FORWARD;
            msg_packet.payload =  message;
            msg_packet.pubkey = item.pkey;
            Packet::encrypt_packet(msg_packet);
            me->forward_packet(msg_packet);

            return true;
        }
    }


    //NACK for source
    if (!found) {
        Color::print_bright_yellow("CONSIDER JOINING FIRST ( • ᴗ - ) ✧");
    }

    Color::print_red("Couldn't send message '" + message + "' to " + dest +  " successfully");

    return false;
}




bool Command_Handler::quit() {

  return me->client_stop();
}



bool Command_Handler::list() {

    std::string groups_string = "";

    for(auto &item : me->groups){

        groups_string.append(item.name + " ");

    }

    Color::print_green(groups_string);

    return true;  
}





bool Command_Handler::nick(std::string ctx) {

    std::istringstream iss(ctx);
    std::string cmd_type, payload;
    iss >> cmd_type >> payload; // Extracting "NICK" and the nickname

    std::string old_nick = me->nickname;
    bool result = me->handle_new_nickname(payload);
    if (result) {
        return my_SSL::rename_file(me->nickname, old_nick);
    } else return false;

}

bool Command_Handler::get_key_pair() {
    
    return my_SSL::get_key_pair(me->nickname);
}



bool Command_Handler::get_public_key(std::string ctx) {

    std::istringstream iss(ctx);
    std::string cmd_type, name;
    iss >> cmd_type >>  name; 
    struct Connected_Client cc;
    if (find_client(name, cc)) {
        Color::print_orange(my_SSL::pubk_toString(cc.pkey));
        return true;
    }

    struct Group g;
    if (find_group(name, g)) {
        Color::print_orange(my_SSL::pubk_toString(g.pkey));
        return true;
    } 
    return false;
    
}





bool Command_Handler::get_help() {
    Color::print_help();
    return true;
}

bool Command_Handler::ls() {
    
    Color::print_bright_cyan("left socket : " + std::to_string(me->left_socket) + ", right socket : " + std::to_string(me->right_socket));
    Color::print_bright_cyan("left : " + std::to_string(me->left) + ", right : " + std::to_string(me->right) + ", mynumber : " + std::to_string(me->my_number));
    Color::print_bright_cyan("multireceiver socket : " + std::to_string(me->multicast_receiver_socket));
    Color::print_bright_cyan("listener socket : " + std::to_string(me->listener_socket));

    return true;
            
}



bool Command_Handler::handle_command(std::string ctx) {
    std::istringstream iss(ctx);
    std::string cmd_type;
    iss >> std::ws >> cmd_type;
    std::transform(cmd_type.begin(), cmd_type.end(), cmd_type.begin(), ::toupper);

    bool result = false;

    if (cmd_type == "/NICK") {
        result = nick(ctx);
    } else if (cmd_type == "/SETTOPIC") {
        result = set_topic(ctx);
    } else if (cmd_type == "/JOIN") {
        result = join(ctx);
    } else if (cmd_type == "/LEAVE") {
        result = leave(ctx);
    } else if (cmd_type == "/GETTOPIC") {
        result = get_topic(ctx);
    } else if (cmd_type == "/QUIT") {
        result = quit();
    } else if (cmd_type == "/MSG") {
        result = msg(ctx);
    } else if (cmd_type == "/LIST") {
        result = list();
    } else if (cmd_type == "/HELP") {
        result = get_help();
    } else if (cmd_type == "/GETMEMBERS") {
        result = get_members(ctx);
    } else if (cmd_type == "/NEIGHBORS") {
        result = neighbors();
    } else if (cmd_type == "/PING") {
        result = ping(ctx);
    } else if (cmd_type == "/ROUTE") {
        result = route(ctx);
    } else if (cmd_type == "/PLOT") {
        result = plot();
    } else if (cmd_type == "/LS") {
        result = ls();
    } else if (cmd_type == "/SHOW") {
        result = show_clients_list();
    } else if (cmd_type == "/GETPUBLICKEY") {
        result = get_public_key(ctx);
    } else if (cmd_type == "/GETKEYPAIR") {
        result = get_key_pair();
    } else {
        Color::print_bright_red("Unknown command");
        return false;
    }

    return result;
}

bool Command_Handler::find_client(std::string name, Connected_Client& client) {
    
    for (auto& item: me->clients_list) {
        if (item.nickname == name) {
            client = item;
            return true;
        }
    }
    return false;
}

bool Command_Handler::find_group(std::string name, struct Group&group) {
    for (auto& item: me->groups) {
        if (item.name == name) {
            group = item;
            return true;
        }
    }
    return false;

}