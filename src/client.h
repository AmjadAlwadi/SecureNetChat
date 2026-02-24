#ifndef CLIENT_H
#define CLIENT_H
#define RESPONSE_PORT 6540
#define MULTICAST_PORT 3456
#define MAX_WRONG_INPUT 20
#define MULTICAST_GROUP "ff02::132b"
#define BEGINPINGSYMBOL '\x1c'
#define ENDPINGSYMBOL '\x1d'

#include <cstring>
#include <iterator>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
#include <thread>
#include <arpa/inet.h>
#include <unordered_map>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <mutex>
#include <ifaddrs.h>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include <sys/resource.h>
#include <openssl/ssl.h>


#include "validity.h"
#include "packet.h"
#include "group.h"
#include "connected_client.h"
#include "color.h"
#include "command.h"
#include "command_handler.h"
#include "my_ssl.h"



class Client {

public:
  
    Client() = default;
    //protocol
    bool handle_handshake(Packet packet, int port);

    bool handshake(Packet packet, int port);

    bool handle_distribute_client_information(Packet packet, int server_socket);

    bool handle_distribute_group(Packet packet, int server_socket);

    bool handle_new_packet(Packet packet, int socket);

    void client_run(int port);

    bool client_stop();
    
    void handle_user_input();
    
    bool handle_new_nickname(std::string new_nickname);

    bool forward_nickname_to_root(Packet packet);

    bool handle_response_to_nickname(Packet packet);

    void handle_decrease_numbers(Packet packet);



    //Routing
    void socket_handler_thread(int socket, bool new_client);

    void handle_disconnection(int socket);

    int init_listener_socket(int port);  // ipv6

    int connect_to_hostname(std::string hostname);  // ipv6

    bool connect_to_root();

    void connect_to_root_thread();
    
    Connected_Client to_connected_client();

    void stop_multicast_thread();

    std::string find_hostname_of_number(int number);



    //Multicast
    int start_multicast_receiver();

    int join_network(int port);

    void start_multicast_receiver_thread_function();

    // General Client functions
    bool receive_packet(Packet* packet, int socket);

    bool send_packet(Packet packet, int socket);

    void distribute_client_information(Connected_Client c);

    void distribute_group_information(Group& group);

    int calculate_number();

    void set_me_as_root();

    bool forward_packet(Packet packet);

    long long timeToMilliseconds(std::time_t& timeValue);

    std::string find_nickname_of_number(int number);

    void decrease_numbers();




    // Attributes
    std::mutex clients_list_mutex;
    std::mutex sockets_mutex;
    std::mutex nickname_mutex;
    int listener_socket = -1;
    int multicast_receiver_socket = -1;
    int multicast_sender_socket = -1;
    int multicast_receive_response_socket = -1;
    int multicast_send_response_socket = -1;
    int right_socket = -1;
    int left_socket = -1;
    int my_number = -1; 
    int right = -1;
    int left = -1;
    int listening_port;
    bool handle_user_input_status = true;
    std::string nickname;
    std::string hostname;
    std::vector<struct Group> groups; // all groups
    std::vector<struct Connected_Client> clients_list;

    EVP_PKEY* pubkey;
};



#endif