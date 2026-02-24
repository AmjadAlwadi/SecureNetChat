cmake_minimum_required(VERSION 3.10)
project(aufgabe_4)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread")

add_executable(client
        client.h
        client.cpp
        validity.h
        validity.cpp
        packet.cpp
        packet.h
        color.cpp
        color.h
        command.h
        connected_client.h
        group.h
        command_handler.h
        command_handler.cpp
        my_ssl.h
        my_ssl.cpp
       
        )

target_link_libraries(client PRIVATE ${OPENSSL_LIBRARIES})
