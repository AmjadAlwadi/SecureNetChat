#ifndef CONNECTED_CLIENT_H
#define CONNECTED_CLIENT_H

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>

struct Connected_Client {

    int number;
    std::string nickname;
    std::string hostname;
    EVP_PKEY* pkey;
};

#endif
