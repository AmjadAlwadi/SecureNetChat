#include "client.h" 

std::vector<std::string> splitString(const std::string& input, const std::string& delimiter) {
    std::vector<std::string> parts;
    std::string tempInput = input; // Make a copy of the input to modify
    size_t pos = 0;
    std::string part;

    while ((pos = tempInput.find(delimiter)) != std::string::npos) {
        part = tempInput.substr(0, pos);
        parts.push_back(part);
        tempInput.erase(0, pos + delimiter.length());
    }

    // Add the last part
    parts.push_back(tempInput);

    return parts;
}

// Hilfsfunktion : gets vector and delimiter and returns string
std::string join_vector(const std::vector<std::string>& vec, const std::string& delimiter) {

    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i != vec.size() - 1) {
            oss << delimiter;
        }
    }
    return oss.str();
}


// Hilfsfunktion : gets string and delimiter and returns vector
std::vector<std::string> split_string(const std::string& str, const std::string& delimiter) {

    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start, end));
    
    return result;
}






// Only in tcp socket
bool Client::send_packet(Packet packet, int socket) {
    std::string new_payload = my_SSL::base64_encode(packet.payload);
    packet.payload = new_payload;
    std::string packet_string = packet.to_string() + "\t";
    
    size_t total_bytes_sent = 0;
    size_t bytes_left_to_send = packet_string.size();
    Color::print_blue("Sent on Socket: " + std::to_string(socket));
    Color::print_blue("Sent: " + packet.print());
    
    while (total_bytes_sent < packet_string.size()) {
        ssize_t bytes_sent = write(socket, packet_string.c_str() + total_bytes_sent, bytes_left_to_send);
        Color::print_green("Sent byte: " + std::to_string(bytes_sent));
        
        if (bytes_sent <= 0) {
            if (bytes_sent == 0) {
                Color::print_bright_red("Socket disconnected");
            } else {
                Color::print_bright_red("Error: Could not send data");
            }
            return false;
        }

        total_bytes_sent += static_cast<size_t>(bytes_sent);
        bytes_left_to_send -= static_cast<size_t>(bytes_sent);
    }

    

    return true;
}


// Function to convert time_t to milliseconds
long long Client::timeToMilliseconds(std::time_t& timeValue) {
    struct timeval tv;
    tv.tv_sec = timeValue;
    tv.tv_usec = 0;
    return static_cast<long long>(tv.tv_sec) * 1000LL + static_cast<long long>(tv.tv_usec) / 1000LL;
}


// Only in tcp socket
bool Client::receive_packet(Packet* packet, int socket) {

    char buffer[MAX_BUFF];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received = 0;
    ssize_t total_bytes_received = 0;

   
    while (total_bytes_received < sizeof(buffer) - 1) {
        bytes_received = read(socket, buffer + total_bytes_received, 1);

        if (bytes_received <= 0) {
            
            std::cerr << "Client disconnected\n";
            handle_disconnection(socket);
            return false;
            
        }

        total_bytes_received += bytes_received;

        if (buffer[total_bytes_received - 1] == '\t') {
            // Found the newline delimiter, consider it the end of the packet

            buffer[total_bytes_received - 1] = '\0';
            *packet = Packet::str_to_packet(std::string(buffer));
            Color::print_blue("Received : " + packet->print());

            try {
                std::string new_payload = my_SSL::base64_decode(packet->payload);
                packet->payload = new_payload;
                
            } catch (const std::runtime_error& e) {
                Color::print_red("decoding went wrong");
            }

            
         
            Color::print_orange("Received decoded : " + packet->print());
           
            return true;
        }
    }
    
    // Handle the case where the packet is too large for the buffer
    std::cerr << "Error : Received data exceeds buffer size\n";
    return false;
}




void Client::connect_to_root_thread(){

    while(1){

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for(auto client : clients_list){

            if(client.number == 0){
                connect_to_root();
                return;
            }
        }
    }
}




bool Client::connect_to_root(){

    if(left != 0){

        close(right_socket);

        // Close the network by connecting to the root
        right_socket = connect_to_hostname(find_hostname_of_number(0));


        if(right_socket < 0){

            Color::print_red("Error : handshake error, socket connection to root error");
            return false;

        }

        right = 0;

        // Sending handshake to root
        Packet handshake_packet_root;
        handshake_packet_root.command = command_enum::HANDSHAKE;
        handshake_packet_root.src = std::to_string(my_number);
        handshake_packet_root.dest = std::to_string(0);
        handshake_packet_root.pubkey = pubkey;

        send_packet(handshake_packet_root, right_socket);

    }

    return true;

}





struct Connected_Client Client::to_connected_client(){


    struct Connected_Client me;
    me.hostname = this->hostname;
    me.nickname = this->nickname;
    me.number = this->my_number;
    me.pkey = my_SSL::get_public_key(this->nickname);

    return me;
}


void Client::start_multicast_receiver_thread_function(){

        if((right == 1 || right == -1) && multicast_receiver_socket == -1){
        
            // Start the multicast receiver for new users 
            std::thread start_multicast_receiver_thread(&Client::start_multicast_receiver, this);

            start_multicast_receiver_thread.detach();

            std::cout << "Multicast receiver started\n";

        }
}






void Client::stop_multicast_thread(){

    if(multicast_receiver_socket != -1){
        Color::print_bright_black("multicast_receiver stopped");
        close(this->multicast_receive_response_socket);
        close(this->multicast_send_response_socket);
        close(this->multicast_receiver_socket);
        this->multicast_receive_response_socket = -1;
        this->multicast_send_response_socket = -1;
        this->multicast_receiver_socket = -1;
    }
}





void Client::distribute_client_information(struct Connected_Client client){

    //dirstibuting server-client informations (topology info)

    std::lock_guard<std::mutex> lock(clients_list_mutex);

    Packet packet;
    packet.command = command_enum::DISTRIBUTE_CLIENT_INFORMATION;
    packet.src = std::to_string(client.number);
    packet.dest = client.hostname;
    packet.payload = client.nickname;
    packet.pubkey = client.pkey;
    
    

    // Send information

    if(left_socket != -1 && my_number != 0){
    
        send_packet(packet,left_socket);


    }

    if(right_socket != -1 && right != 0){

        send_packet(packet,right_socket);
        
    } 


    bool found = false;


    // Apply that information locally

    // Check if client nickname is empty and then delete it in our list if that's the case
    if(client.nickname.empty()){

        
        for(int i = 0;i<clients_list.size();i++){
            
            if(clients_list.at(i).hostname == client.hostname){

                clients_list.erase(clients_list.begin() + i);
                break;

            }

        }


    }else{
        
        // Change nickname
        for(auto& item: clients_list){
            
            if(item.hostname == client.hostname){
                item.nickname = client.nickname;
                item.number = client.number;
                item.pkey = client.pkey;
                found = true;
            }  
        }

        if(hostname == client.hostname){
            this->nickname = client.nickname;
            this->my_number = client.number;
        }
        
        if(!found) clients_list.push_back(client); 

    }   

    if(my_number == 0 && calculate_number() - 1 != left && left > 1){
        left = calculate_number() - 1;
    }

}


bool Client::handle_distribute_client_information(Packet packet, int socket){


    // Forward
    if(socket == left_socket){
        
        if(right_socket != -1 && right != 0) send_packet(packet,right_socket);

    }else if(socket == right_socket){

        if(left_socket != -1 && my_number != 0) send_packet(packet,left_socket);
        
    }


    bool found = false;

    // Extract and save
    std::string hostname_from_packet = packet.dest;
    std::string client_nickname = packet.payload;
    int client_number = std::stoi(packet.src);

    // Delete client
    if(client_nickname.empty()){

       for(int i = 0;i<clients_list.size();i++){

           if(clients_list.at(i).hostname == hostname_from_packet){

               clients_list.erase(clients_list.begin() + i);
               break;
           }
       }

    }

    
    else{

       // Change nickname
       for(auto& client: clients_list){
           
           if(client.hostname == hostname_from_packet){

               client.nickname = client_nickname;
               client.number = client_number;
               found = true;

           }
       }

        

       if(this->hostname.compare(hostname_from_packet) == 0){

            this->nickname = client_nickname;
            this->my_number = client_number;

        }

        if(!found){

            // Create new connected_client
            Connected_Client new_client;
            new_client.hostname = hostname_from_packet;
            new_client.nickname = client_nickname;
            new_client.number = client_number;
            new_client.pkey = packet.pubkey;

            Color::print_bright_yellow("added new client");
            //Color::print_bright_yellow(client_nickname);
            // Color::print_bright_yellow(my_SSL::pubk_toString(new_client.pkey));
                
            clients_list.push_back(new_client);
        }

    }

    if(my_number == 0 && calculate_number() - 1 != left && left > 1){
        left = calculate_number() - 1;
    }


    return true;

}



void Client::distribute_group_information(struct Group& group){

    // Creating packet and sending it

    Packet packet;
    packet.command = command_enum::DISTRIBUTE_GROUP_INFORMATION;
    packet.src = group.name;
    packet.dest = group.topic;
    packet.payload = group.owner_nickname + ";" +join_vector(group.members_nicknames,";");
    packet.pubkey = group.pkey;


    if(left_socket != -1 && my_number != 0){

        send_packet(packet,left_socket);

    }

    if(right_socket != -1 && right != 0){

        send_packet(packet,right_socket);

    }



    // Check if group members is empty

    if(group.members_nicknames.size() == 0){

        packet.payload = "";


        // Check if exists
        int index = 0;

        for(auto& item : this->groups){

            if(group.name == item.name){

                break;
            }

            index++;
        }


        // Delete group

        this->groups.erase(this->groups.begin() + index);


        

    }else{

        // Check if exists

        for(struct Group& item : this->groups){

            if(group.name == item.name){

                item.members_nicknames = group.members_nicknames;
                item.topic = group.topic;
                
                return;
            }
        }

        //if not then

        struct Group new_group;
        new_group.name = group.name;
        new_group.owner_nickname = group.owner_nickname;
        new_group.topic = group.topic;
        new_group.members_nicknames = group.members_nicknames;
        new_group.pkey = group.pkey;

        this->groups.push_back(new_group);


    }

}



bool Client::handle_distribute_group(Packet packet, int socket){

    // Forward
    if(socket == left_socket){

        if(right_socket != -1 && right != 0) send_packet(packet,right_socket);

    }else if(socket == right_socket){
        
        if(left_socket != -1 && my_number != 0) send_packet(packet,left_socket);
        
    }else{
    
        Color::print_red("Error : couldn't distribute information about new group");
        return false;
    }



    // Extracting information

    struct Group new_group;

    new_group.name = packet.src;
    new_group.topic = packet.dest;
    new_group.pkey = packet.pubkey;


    // If empty

    if(packet.payload.empty()){

        // Check if exists
        
        int index = 0;

        for(auto& item : this->groups){

            if(new_group.name == item.name){

                break;

            }

            index++;
        }


        // Delete group
        

        this->groups.erase(this->groups.begin() + index);



    }else{

        std::vector<std::string> group_vector = split_string(packet.payload,";");

        new_group.owner_nickname = group_vector.at(0);
        group_vector.erase(group_vector.begin());
        new_group.members_nicknames = group_vector;   


        // Check if exists

        for(auto& item : this->groups){

            if(new_group.name == item.name){

                item.members_nicknames = new_group.members_nicknames;
                item.topic = new_group.topic;
                return true;

            }
        }



        // Creating new group if not exists

        this->groups.push_back(new_group);

    }


    return true;

}



bool Client::forward_packet(Packet packet){

    Connected_Client target;
    int number;

    //find client
    for(auto& item : clients_list){

        number = -1;
    
        if(item.nickname == packet.dest){
            number = item.number;
            break;
        }

    }

    // Not found
    if(number == -1){

        Color::print_red("Error : client not found to forward to");
        return false;
    }


    // It's me
    if(number == my_number){

        
        // Means ping request
        if(packet.payload[0] == BEGINPINGSYMBOL){
            std::string temp = packet.dest;
            packet.dest = packet.src;
            packet.src = temp;
            packet.payload.at(0) = ENDPINGSYMBOL;
            forward_packet(packet);
        }


        // Means ping response arrived
        else if(packet.payload[0] == ENDPINGSYMBOL){

            struct timeval tv;

            gettimeofday(&tv, nullptr); // Get the current time in microseconds

            long long milliseconds = static_cast<long long>(tv.tv_sec) * 1000LL + static_cast<long long>(tv.tv_usec) / 1000LL;

            long long diff = milliseconds - std::stoll(packet.payload.substr(1));

            Color::print_green("Rtt from " + packet.dest + " to " + packet.src + " : " + std::to_string(diff) + "ms");
            
            return true;
        }
        else if (packet.src.find("|") != std::string::npos) {
            std::vector<std::string> names = splitString(packet.src, "|");
            for (auto &g :this->groups) {
                if (g.name == names[0]) {
                    struct Group_Secret gs;
                    gs.nickname = names[1];
                    gs.key = packet.payload;
                    g.secrets.push_back(gs);
                    Color::print_cyan(names[1] +" secret added for group " + names[0]  + gs.key);
                }
            } 
        }
        else{
            /*
            "Because the decryption would fail or the packet may not be intended for this client,
            the decryption process cannot occur within the packet class. 
            Only the client with the internally stored nickname can load the private key and decrypt the packet.
            Additionally, the payload of the packet should not change, as we might need to forward the packet."
            */
            if (my_SSL::compare_evp_pkeys(packet.pubkey, pubkey)) {

                Color::print_orange("public key verified");
                std::string plaintext = my_SSL::decrypt(packet.payload, nickname); 
                Color::print_green(packet.src + " : " + plaintext);
                return true;
            } else {
                Color::print_red("key did not verified");
            }
            for (auto & g: groups) {
                if (my_SSL::compare_evp_pkeys(g.pkey, packet.pubkey)) {
                    Color::print_orange("public group key verified");

                    for(auto& gs: g.secrets) {
                        if (gs.nickname == packet.src) {
                            std::string plaintext = my_SSL::aes_decrypt(packet.payload, reinterpret_cast<const unsigned char*>(gs.key.c_str()));
                            Color::print_green(packet.src + " : " + plaintext);
                        }

                    }
                   
                    
                } else {
                    Color::print_red("group key did not verified");
                }
            }
        }

        return true;
    
    }


    int distance = std::abs(my_number - number);

    // With or without rotation
    int distance_right_with = calculate_number() - 1 - my_number + (number + 1) ;
    int distance_right_without = distance;
    int distance_left_with = my_number + calculate_number() - 1 - number + 1 ;
    int distance_left_without = distance;



    // Forward to the left client
    if(number < my_number){

        if(distance_left_without <= distance_right_with){
            send_packet(packet, left_socket);
        }
        else{
            send_packet(packet, right_socket);
        }
    }


    // Forward to the right server
    if(number > my_number){

        if(distance_right_without <= distance_left_with){
            send_packet(packet, right_socket);
        }
        else{
            send_packet(packet, left_socket);
        }
    }

    return true;

 }





int Client::calculate_number(){

    int last_node = 0;  
    
    // Find the last node in the topology
    for(auto client : clients_list){

        if(client.number > last_node){

            last_node = client.number;

        }

    }

    return last_node + 1;

}

std::string Client::find_nickname_of_number(int number){

     for(auto client : clients_list){

        if(client.number == number){

            return client.nickname;

        }

    }

    return "";
}



std::string Client::find_hostname_of_number(int number){


    for(auto client : clients_list){

        if(client.number == number){

            return client.hostname;

        }

    }

    return "";
}





//######################################################//

//######################################################//

//######################################################//

//######################################################//

//######################################################//

//######################################################//

//######################################################//

//######################################################//
            //The Beginnin after the end//
//######################################################//

//######################################################//

//######################################################//

//######################################################//

//######################################################//

//######################################################//

//######################################################//

//######################################################//




int Client::start_multicast_receiver(){

    // Create socket for sending response

    struct sockaddr_in6 server_addr;

    // Create a UDP socket
    multicast_send_response_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (multicast_send_response_socket < 0) {
        close(multicast_receiver_socket);
        close(multicast_send_response_socket);
        multicast_send_response_socket = -1;
        multicast_receiver_socket = -1;
        perror("socket");
        exit(1);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(RESPONSE_PORT);


    // Create multicast receiver socket

    struct sockaddr_in6 multicast_addr;
    char buffer[MAX_BUFF];


    // Create a socket
    multicast_receiver_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (multicast_receiver_socket < 0) {
        close(multicast_receiver_socket);
        close(multicast_send_response_socket);
        multicast_send_response_socket = -1;
        multicast_receiver_socket = -1;
        perror("socket");
        exit(1);
    }




    // Set up multicast address
    memset(&multicast_addr, 0, sizeof(multicast_addr));


    multicast_addr.sin6_family = AF_INET6;
    multicast_addr.sin6_port = htons(MULTICAST_PORT);



    // Join the multicast group
    struct ipv6_mreq mreq;
    inet_pton(AF_INET6, MULTICAST_GROUP, &(mreq.ipv6mr_multiaddr));
    mreq.ipv6mr_interface = 0;  // Use the default interface
    if (setsockopt(multicast_receiver_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) < 0) {
        close(multicast_receiver_socket);
        close(multicast_send_response_socket);
        multicast_send_response_socket = -1;
        multicast_receiver_socket = -1;
        perror("setsockopt");
        exit(1);
    }

    int enable = 1;
    if (setsockopt(multicast_receiver_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }


    // Bind the socket to the multicast address
    if (bind(multicast_receiver_socket, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
        close(multicast_receiver_socket);
        close(multicast_send_response_socket);
        multicast_send_response_socket = -1;
        multicast_receiver_socket = -1;
        perror("bind");
        exit(1);
    }




    struct sockaddr_in6 sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    


    while (multicast_receiver_socket >= 0) {

        // Receive packet
        memset(buffer,0,sizeof(buffer));

        if(multicast_receiver_socket == -1){
            return 0;
        }

        ssize_t len = recvfrom(multicast_receiver_socket, buffer, sizeof(buffer), 0,(struct sockaddr*)&sender_addr, &sender_addr_len);

        if(multicast_receiver_socket < 0){
            close(multicast_receiver_socket);
            close(multicast_send_response_socket);
            multicast_send_response_socket = -1;
            multicast_receiver_socket = -1;
            return 0;
        }

        std::string cpp_string(buffer);

        Packet packet = Packet::str_to_packet(cpp_string);
        
        // If not handshake then reject
        if(static_cast<int> (packet.command) != 1){
            
            continue;
        }

        Color::print_blue("Received : " + packet.print());


        // Get the ipv6
        char sender_ip[INET6_ADDRSTRLEN];

        if (inet_ntop(AF_INET6, &(sender_addr.sin6_addr), sender_ip, sizeof(sender_ip)) == NULL) {

            perror("Error : failed to get the ip\n");
            continue;
        }


        Color::print_bright_cyan("Sending response to : " + std::string(sender_ip));

        // Bind the ip of the sender to the socket
        if (inet_pton(AF_INET6, sender_ip, &server_addr.sin6_addr) < 0) {
            perror("inet_pton");
            close(multicast_receiver_socket);
            close(multicast_send_response_socket);
            multicast_send_response_socket = -1;
            multicast_receiver_socket = -1;
            exit(1);
        }


        // Caclulate the number
        int other_number =  this->calculate_number();


        if(other_number < 0){

            continue;

        }

        

        // Create socket and send response
        Packet sent_packet;
        sent_packet.command = command_enum::HANDSHAKE;
        sent_packet.src = std::to_string(other_number - 1);
        sent_packet.dest = std::to_string(other_number);
        sent_packet.pubkey = pubkey;


        if (other_number >= 2) {
            int last_node = 0;
            std::string last_host = "";  
        
        // Find the last node in the topology
        for(auto client : clients_list){

            if(client.number > last_node - 1 && client.hostname != ""){

                last_node = client.number;
                last_host = client.hostname;

            }
        }
        std::cout << "connect to: "<< last_host << std::endl;

        sent_packet.payload = std::string(last_host);
        } else {
            sent_packet.payload = std::string(hostname);
        }

        if(Validity::is_valid_nickname(packet.payload)){

            for(auto item : clients_list){
                if(packet.payload == item.nickname){
                    sent_packet.payload = "Name is already in use!";
                }
            }    
        }



        // Send data to the server
        ssize_t bytes_sent = sendto(multicast_send_response_socket, sent_packet.to_string().c_str(), sent_packet.to_string().size(), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        Color::print_cyan("Sent handshake to : " + sent_packet.print());

        if (bytes_sent < 0) {
            perror("sendto");
            close(multicast_receiver_socket);
            close(multicast_send_response_socket);
            multicast_send_response_socket = -1;
            multicast_receiver_socket = -1;
            exit(1);
        }

    }

    return 0;

}








// Send hi message
// Receive response
// Run handshake on it
// Connect to the given hostname
// Set the number and socket
// Sending handshake to previous
// Run a thread to connect to root whenever possible
// Send info about me

int Client::join_network(int port){


    // Create socket for receiving the response

    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[MAX_BUFF];

    // Create a UDP socket
    int multicast_receive_response_socket= socket(AF_INET6, SOCK_DGRAM, 0);
    if (multicast_receive_response_socket < 0) {
        close(multicast_sender_socket);
        close(multicast_receive_response_socket);
        perror("socket");
        exit(1);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(RESPONSE_PORT);
    server_addr.sin6_addr = in6addr_any;  // Listen on all available interfaces

    // Bind the socket to the server address
    if (bind(multicast_receive_response_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(multicast_sender_socket);
        close(multicast_receive_response_socket);
        perror("bind");
        exit(1);
    }




    // Create multicast sender socket

    struct sockaddr_in6 multicast_addr;
    socklen_t multicast_addr_len = sizeof(multicast_addr);

    // Create a socket
    multicast_sender_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (multicast_sender_socket < 0) {
        close(multicast_sender_socket);
        close(multicast_receive_response_socket);
        perror("socket");
        exit(1);
    }


    // Set up multicast address
    memset(&multicast_addr, 0, sizeof(multicast_addr));

    multicast_addr.sin6_family = AF_INET6;
    multicast_addr.sin6_port = htons(MULTICAST_PORT);

    inet_pton(AF_INET6, MULTICAST_GROUP, &(multicast_addr.sin6_addr));


    // Construct packet to send and do the handshake
    Packet sent_packet;
    sent_packet.command = command_enum::HANDSHAKE;
    sent_packet.src = this->hostname;
    sent_packet.payload = this->nickname;
    sent_packet.pubkey = pubkey;

    std::string message = sent_packet.to_string();


    // Send multicast message
    if (sendto(multicast_sender_socket, message.c_str(), message.size(), 0, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
        close(multicast_sender_socket);
        close(multicast_receive_response_socket);
        perror("sendto");
        exit(1);
    }

    
    // Receive the response of the sent message

    memset(buffer,0,sizeof(buffer));


    // Receive data from the client
    ssize_t bytes_received = recvfrom(multicast_receive_response_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);



    if (bytes_received < 0) {
        close(multicast_sender_socket);
        close(multicast_receive_response_socket);
        perror("recvfrom");
        return -1;
    }

    std::string cpp_string(buffer);

    Packet received_packet = Packet::str_to_packet(cpp_string);

    Color::print_blue("Received : " + received_packet.print()); 

    if (received_packet.payload.compare("Name is already in use!") != 0){

        handshake(received_packet, port);

    } else {
        Color::print_red("Name is already in use!");
    }

    


    close(multicast_sender_socket);
    close(multicast_receive_response_socket);

    return 0;

}









int Client::connect_to_hostname(std::string chostname){

    struct addrinfo hints, *res, *p;
    int sockfd;

    // Set up the hints structure
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6; // Request IPv6 addresses
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;

    Color::print_bright_cyan("Connecting to " + chostname + " on port " + std::to_string(listening_port));

    // Resolve the hostname to IPv6 addresses
    if (getaddrinfo(chostname.c_str(), std::to_string(listening_port).c_str(), &hints, &res) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    // Loop through the results and connect to the first successful one
    for (p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("connect");
            continue;
        }

        // Successfully connected
        break;
    }

    freeaddrinfo(res); // Free the result structure

    if (p == nullptr) {
        std::cerr << "Failed to connect to " << chostname << " on port " << listening_port << std::endl;
        return -1;
    }

    Color::print_green("Connected to the server at " + chostname + " on port " + std::to_string(listening_port));



    // Start the socket handler thread
    std::thread new_socket_handler_thread(&Client::socket_handler_thread, this, sockfd, true);
    new_socket_handler_thread.detach();

    return sockfd;
}







// Start the listener socket
int Client::init_listener_socket(int port){

    // Create a socket
    if ((listener_socket = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
        
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the server address structuree
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);

   
    // Bind the socket to the specified port
    if (bind(listener_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        close(listener_socket);
        listener_socket = -1;
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(listener_socket, 30) == -1) {
        perror("Error listening on socket");
        close(listener_socket);
        listener_socket = -1;
        exit(EXIT_FAILURE);
    }

    Color::print_bright_green("Server listening on port : " + std::to_string(port));


    while(listener_socket >= 0){

        // Accept a connection

        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

       
        int new_socket = accept(listener_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        Color::print_bright_green("New connection accepted");
        if (new_socket == -1) {

            Color::print_red("Error accepting connection");

            if(errno == EBADF){
                close(listener_socket);
                listener_socket = -1;
            }
            continue;  
        }


        // Start the socket handler thread
        std::thread new_socket_handler_thread(&Client::socket_handler_thread, this, new_socket, false);

        new_socket_handler_thread.detach();

    }


    return 0;
}
    




void Client::socket_handler_thread(int socket, bool new_client){


    if(!new_client){

        // Receive the handshake
    
        Packet handshake_packet;

        Color::print_bright_blue("trying to add the client");

        if(!receive_packet(&handshake_packet,socket)){

            Color::print_red("Error : hanshake receive error");
            return;

        }

        if(handshake_packet.command == command_enum::DISTRIBUTE_CLIENT_INFORMATION){
            
            if(!receive_packet(&handshake_packet,socket)){

            Color::print_red("Error : hanshake receive error");
            return;

            }
        }


        if(handshake_packet.command != command_enum::HANDSHAKE){

            Color::print_red("Error : handshake went wrong");
            return;

        }

        if(!handle_handshake(handshake_packet,socket)){

            Color::print_red("Error : handshake handle error");
            return;
        }

        int other_number = std::stoi(handshake_packet.src);

         Color::print_bright_blue("trying to add the client");

        
    
        // Only to do when not root and information not already there

        bool found = false;

        for(auto item : clients_list){
            if(item.number == other_number){
                found = true;
            }
        }


        if(!found){


                // Receive neighbour info
            
                Packet client_packet;

                if(!receive_packet(&client_packet,socket)){

                    Color::print_red("Error : receive packet error");

                }

                if(client_packet.command != command_enum::DISTRIBUTE_CLIENT_INFORMATION){

                    Color::print_red("Error : handshake went wrong, no distribute packet");
                    return;

                }

                if(!handle_distribute_client_information(client_packet,socket)){

                    Color::print_red("Error : handshake error on distribute handle");
                    return;
                }
                //Color::print_yellow(client_packet.to_string());

                
            // Add me to list
            distribute_client_information(this->to_connected_client());

            // Send all clients information to new client only if im not root
            for(auto client : clients_list){

                distribute_client_information(client);

            }

        }
        
           
    }


   
    // Receive and handle
    while(1){   

        // Receive packet
        Packet received_packet;

        if(!receive_packet(&received_packet,socket)){

            break;

        }

        // Handle it
        handle_new_packet(received_packet,socket);

    }
}







bool Client::handshake(Packet packet, int port){
    

    // Connect to the given hostname
    int socket = connect_to_hostname(packet.payload);

    if(socket < 0){

        Color::print_red("Error : handshake error, socket connection to node error");
        return false;

    }

    // Set the number and socket
    
    try {
        my_number = std::stoi(packet.dest);
    } catch (const std::exception& e) {
        Color::print_red("Error: Could not parse the destination number: " + std::string(e.what()));
        return false;
    }

    int other_number = std::stoi(packet.src);


    if (other_number < my_number) {
    
    if (left_socket != -1) {
        close(left_socket);
    }
    left = other_number;
    left_socket = socket;
    
    }

    else {

        Color::print_red("Error : handshake error, got wrong number");
        return false;
    }


    // Sending handshake to previous
    Packet handshake_packet;
    handshake_packet.command = command_enum::HANDSHAKE;
    handshake_packet.src = std::to_string(my_number);
    handshake_packet.dest = std::to_string(other_number);
    handshake_packet.pubkey= pubkey;

    send_packet(handshake_packet, left_socket); 



    // Run a thread to connect to root whenever possible

    std::thread root_connector_thread(&Client::connect_to_root_thread,this);
    root_connector_thread.detach();


    // Send info about me
    distribute_client_information(this->to_connected_client());
    
    client_run(port);

    return true;

}







bool Client::handle_handshake(Packet packet, int socket){
    

    // Set the number and socket
    if (right_socket == -1 && my_number == 0 && left_socket != -1){

        right = left;
        left = -1;
        right_socket = left_socket;
        left_socket = -1;

        Packet decrease_right;
        decrease_right.command = command_enum::DECREASE;
        decrease_right.src = std::to_string(my_number);
        decrease_right.pubkey = this->pubkey;

        send_packet(decrease_right, socket);

    }

    my_number = std::stoi(packet.dest);
    int other_number = std::stoi(packet.src);


    if(other_number > my_number && my_number == 0 && other_number > 1 && right_socket != -1){
        if (left_socket != -1) {
            close(left_socket);
        }

        left = other_number;
        left_socket = socket;

    }

    else if (other_number > my_number) {
        
        if (right_socket != -1) {
            close(right_socket);
        }
        if (right == 0) right = other_number;
        right = other_number;
        right_socket = socket;

    }

    else if(other_number < my_number && left_socket == -1){

        left = other_number;
        left_socket = socket;
    }

    else {
        Color::print_red("Error : handshake error, got wrong number");
        return false;
    }


    return true;
}








void Client::handle_disconnection(int socket){


    Color::print_magenta("Handling the disconnection of the neighbor node");


    close(socket);

    // Find last node

    int last_number = calculate_number() - 1;

    int other_number = right + 1;

    // Find client to delete from the list
    Connected_Client client;

    for(auto item : clients_list){

        if(item.number == right){
            client = item;
        }
    }


    client.nickname = "";
   
    if(socket == left_socket){

        Color::print_magenta("Case left");

        // Switch sockets (1 disconnected)
        if(my_number == last_number && last_number == 2 && left == 1){
            Color::print_magenta("Case 1");

            // my_number = 1;
            left = 0; 
            if (left_socket != -1) {
                close(left_socket);
                left_socket = -1;
            }
            left_socket = right_socket;
            right_socket = -1;
            right = -1;
           
        }


        // Switch sockets (last disconnected)
        else if(my_number == 0 && last_number == 2 && right == 2){
            Color::print_magenta("Case 2");
            left = -1;
            left_socket = -1;
            right = -1;
            right_socket = -1;
            
        }
        

        // Switch sockets (root disconnected)
        else if(my_number == 1 && last_number >= 2){
            Color::print_magenta("Case 3");
            right = -1;
            left = 0;
            left_socket = -1;
            right_socket = -1;
        }

        else if(right == -1){
            Color::print_magenta("Case 4");
            my_number = 0;
            left = -1;
            left_socket = -1;
            start_multicast_receiver_thread_function();
            distribute_client_information(this->to_connected_client());
        }

        else{

            Color::print_magenta("Case 5");
            this->left = -1;
            this->left_socket = -1;
            if (left == 0){
                for (auto& item : clients_list){
                    if (item.number == 0){
                        item.nickname = "";
                        
                        distribute_client_information(item);  
                    }
                }
                right = -1;
                my_number = 0;
                
                start_multicast_receiver_thread_function();
                 
                distribute_client_information(this->to_connected_client()); 
                this->left = -1;
            }

        }

        // Delete root from list
        if(right == -1 && left == -1){

            for(auto item : clients_list){

                if(item.number == 0){
                    client = item;
                }
            }

            client.nickname = "";
            distribute_client_information(client);

        }

        distribute_client_information(this->to_connected_client());
        
        return;
    }

    if(socket != right_socket) return;

    
    Color::print_magenta("Case right");


    // It's the right socket
    

    // I need to connect to the root
    if(my_number == last_number - 1){
        Color::print_magenta("case 0");
        right = -1;
        right_socket = -1;
        other_number = 0;
    }

    // I need to connect to 1
    else if(my_number == last_number){
        Color::print_magenta("case root");

        other_number = 1;
        this->my_number = 0;
        this->left = -1;
        if (last_number <= 2){
            left_socket = -1;
        }
        
        start_multicast_receiver_thread_function();

    }


    // New if branch



    if(my_number == other_number){

        right_socket = -1;
        left = -1;
        right = -1;
    }


    // Odd situations that happen if there are only 3 clients

    else if(other_number == left && my_number == last_number - 1 && other_number == 0){
        Color::print_magenta("Case 1");

        right_socket = -1;
        left = 0;
        right = -1;

    }


    else if(other_number == left && my_number == 0 && other_number == last_number){

        Color::print_magenta("Case 2");
        // Exchange the sockets
        right = 1;
        if (right_socket != -1) {
            close(right_socket);
            right_socket = -1;
        }
        right_socket= left_socket;
        left_socket = -1;
        left = -1;
        
    }

    else if(other_number == left && my_number == 0 && other_number == 1){

        Color::print_magenta("Case 3");
        right = -1;
        if (right_socket != -1) {
            close(right_socket);
            right_socket = -1;
        }
        left_socket = -1;
        left = -1;
    
    // }else if (last_number <= 2){
    //     Color::print_magenta("Case 4");
    //     this->right_socket = -1;
    //     this->right = -1; 

    } else{

        // Repair the connection
        this->right_socket = connect_to_hostname(find_hostname_of_number(other_number));
        this->right = other_number;
        std::cout << right;

        // Sending handshake
        Packet handshake_packet;
        handshake_packet.command = command_enum::HANDSHAKE;
        handshake_packet.src = std::to_string(my_number);
        handshake_packet.dest = std::to_string(other_number);
        handshake_packet.payload = this->nickname;
        handshake_packet.pubkey = pubkey;

        send_packet(handshake_packet, this->right_socket); 

    }

    // Start multicast thread

    if(right == 1 || right_socket == -1 || my_number == 0){
        start_multicast_receiver_thread_function();
    }


    // Delete the client in the topology

    distribute_client_information(client);

    distribute_client_information(this->to_connected_client());


    // To know whether root is affected or not
    bool change = false;


    if(other_number == 1 && last_number > 2){
        
        set_me_as_root();
        
        Packet decrease_right;
        decrease_right.command = command_enum::DECREASE;
        decrease_right.src = std::to_string(my_number);

        send_packet(decrease_right, left_socket);


    }else if(other_number != 1 && right != -1){
        
        decrease_numbers();
    }

    return;

}



void Client::decrease_numbers(){
    
    // Forward
    int last = calculate_number() - 2;
  
    if (right != 0){
        this->right = this->my_number != last? this->my_number + 1 : 0;
    }
    Packet dec_packet;
    dec_packet.command = command_enum::DECREASE;
    dec_packet.src = std::to_string(my_number);
    dec_packet.pubkey = this->pubkey;
    
    if(right != 0) {
        
        send_packet(dec_packet,right_socket);
    }
    
}


void Client::handle_decrease_numbers(Packet packet){

    int last = calculate_number() - 2;
    int from_client = std::stoi(packet.src);

    if (my_number == last && from_client == 0){
        right = 0;
        distribute_client_information(this->to_connected_client());
        return;
    }

    if (my_number > from_client && my_number != 0){
        this->my_number --;
        this->right = this->my_number != last? this->my_number + 1 : 0;
        this->left = this->my_number != 0? this->my_number - 1 : last;
    }

    if (my_number == 1 && last == 1){
        this->right = -1;
    }



    distribute_client_information(this->to_connected_client());
    // Forward

    packet.command = command_enum::DECREASE;

    if(right_socket != -1 && my_number != 0) send_packet(packet,right_socket);

}




void Client::set_me_as_root(){

    this->my_number = 0;
    this->right = 1;
    start_multicast_receiver_thread_function();
    distribute_client_information(this->to_connected_client());

    // // Start multicast receiver on the last node
    // Packet packet;
    // packet.command = command_enum::STARTMULTICAST;
    // send_packet(packet,left_socket);

}



// Nickname functions



bool Client::handle_new_nickname(std::string new_nickname) {

    if (Validity::is_valid_nickname(new_nickname)) {


        // If nickname is valid then take that nickname to the root server and handle it in a specific case (command_enum case)
        Packet packet_to_last;
        packet_to_last.command = command_enum::NICK;
        packet_to_last.src = this->nickname;
        packet_to_last.payload = new_nickname;

        std::string last;

        for(auto item : clients_list){
            if(item.number == calculate_number() - 1){
                last = item.nickname;
            }
        }

        packet_to_last.dest = last;

        return forward_nickname_to_root(packet_to_last);  

    } else {

        // Invalid nickname, acknowledge with an error message
        Color::print_red("Error: Invalid nickname format.\nhint:Nicknames could be up to nine characters long and consist of letters or numbers.");
        return false;
    }
    
}






bool Client::forward_nickname_to_root(Packet packet) {

    // Not last node
    if(this->my_number != calculate_number() - 1){
        forward_packet(packet);
        return true;
    }

    // last node
    else{

        Color::print_bright_cyan("Checking nickname...");

        // Validity of the nickname should be checked
        std::lock_guard<std::mutex> lock(nickname_mutex);

        std::string hostname_of_client;

        for (auto item : clients_list){

            if(item.nickname == packet.src){

                hostname_of_client = item.hostname;
            }
        }

        std::string temp_nickname;
        temp_nickname = packet.src;
        packet.src = this->nickname;
        packet.dest = packet.payload;


        packet.command = command_enum::NICKACK;

        for (auto item : clients_list){

            if (item.nickname == packet.payload && item.hostname != hostname_of_client)
            {
                packet.command = command_enum::NICKNACK;
                packet.dest = temp_nickname;
                break;  
            }
        }


        if(packet.command == command_enum::NICKACK){

            for (auto client_item : clients_list){
                if(client_item.nickname == temp_nickname){
                    // Set the new name and push it in the list clients_list
                    client_item.nickname = packet.payload;
                    // Distribute the new info
                    distribute_client_information(client_item);

                    for (auto& item : groups){
                        for (auto& obj : item.members_nicknames){
                            if(obj == temp_nickname){
                                obj = packet.payload;
                                distribute_group_information(item);
                            }
                        }
                        if(item.owner_nickname == temp_nickname){
                                item.owner_nickname = packet.payload;
                                distribute_group_information(item);
                        }
                    }
                    
                    break;
                }
            }
        }
       
       return handle_response_to_nickname(packet);
            
    }     
}






bool Client::handle_response_to_nickname(Packet packet){


    if(packet.dest != this->nickname){
        forward_packet(packet);
        return true;
    }


    if (packet.command == command_enum::NICKACK){

        Color::print_green("Nickname has been set!");


        return true;

    }

    else if (packet.command == command_enum::NICKNACK) 
    {
        Color::print_red("Nickname already exists!");
        return true;
        
    }

    else{
        return true;
    }

    return true;
}











void Client::client_run(int port){

    
    // Listener socket
    // Accept new connections
    // Add_me_to_clients_list
    // Receive the handshake
    // Send all clients information to the new client
    // Receive and handle

    // Start the listener socket thread
    std::thread listener_socket_thread(&Client::init_listener_socket, this, port);

    listener_socket_thread.detach();

    std::cout << "Listening...\n";
    // Receive packet
    // Caclulate the number
    // Send response to handshake
    
    // Start the multicast receiver for new users 
    if (my_number == 0){
        std::thread start_multicast_receiver_thread(&Client::start_multicast_receiver, this);
        start_multicast_receiver_thread.detach();
    }

    std::cout << "Multicast receiver started\n";

    // In the main thread handle user input
    std::thread user_input_thread(&Client::handle_user_input, this);
    user_input_thread.join();
}







// Check for all commands and run it's handler
bool Client::handle_new_packet(Packet packet, int socket){
    switch(packet.command){

        case command_enum::HANDSHAKE:
            return handle_handshake(packet, socket);   
            break;

        case command_enum::DISTRIBUTE_CLIENT_INFORMATION:
            return handle_distribute_client_information(packet, socket);   
            break;

        case command_enum::DISTRIBUTE_GROUP_INFORMATION:
            return handle_distribute_group(packet, socket);   
            break; 


        case command_enum::FORWARD:{
            
            forward_packet(packet);
            break;
        }

        case command_enum::STARTMULTICAST:
            right = 1;
            start_multicast_receiver_thread_function();
            break;

        case command_enum::DECREASE:
            handle_decrease_numbers(packet);
            break;

        case command_enum::NICK:
            forward_nickname_to_root(packet);
            break;

        case command_enum::NICKNACK:
            handle_response_to_nickname(packet);
            break;

        case command_enum::NICKACK:
            handle_response_to_nickname(packet);
            break;
        
        case command_enum::INVALID:
            Color::print_red("invalid packet");
            break; 
        
    }

    return true;

} 


void Client::handle_user_input() {

    Color::print_large_ascii_smiley();
    Command_Handler command_handler;
    command_handler.me = this;

   
    while(handle_user_input_status){
       

        std::cout << "Command : ";
        std::string input;

        std::getline(std::cin, input);    
    
           
        if(!Validity::is_valid_input(input)) {

        } else {
            if (!command_handler.handle_command(input)) {
                Color::print_bright_red("something went wrong executing your command");
            }
        }
    }
}



bool Client::client_stop() {
    close(listener_socket);
    close(multicast_receiver_socket);
    close(multicast_sender_socket);
    close(multicast_receive_response_socket);
    close(multicast_send_response_socket);
    close(right_socket);
    close(left_socket);

    // Optionally set the member variables to -1
    listener_socket = -1;
    multicast_receiver_socket = -1;
    multicast_sender_socket = -1;
    multicast_receive_response_socket = -1;
    multicast_send_response_socket = -1;
    right_socket = -1;
    left_socket = -1;

    handle_user_input_status =false;

    return !handle_user_input_status;
}



int main(int argc, char *argv[]) {

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <nickname> <port> <is_root>\n";
        return -1;
    }

    
    int port = atoi(argv[2]);

    bool root = false;

    if(strcmp(argv[3], "true") == 0) root = true;

    

    // Get own hostname

    char buffer[256];

    if (gethostname(buffer, sizeof(buffer)) == 0) {

    } else {
        perror("gethostname");
        return 1;
    }


    // Check nickname

    std::string desired_nickname = std::string(argv[1]);

    if(!Validity::is_valid_nickname(desired_nickname)){
        Color::print_red("Nickname does not meet the conditions.");
        return 1;
    }

    // Create client

    Client me;

    // Check the nickname to be unique
    me.nickname = desired_nickname;

    me.listening_port = port;

    me.hostname = std::string(buffer);

   
    EVP_PKEY* pk = my_SSL::get_public_key(me.nickname);
    if(!pk) {
        Color::print_red("key and certificate went wrong");
        return 1;
    } else {
        me.pubkey = pk;
    }
    



    // Run client

    if(root){

        me.my_number = 0;
        me.client_run(port);

    }else{
        
        me.join_network(port);
    }

    

    return 0;
}
