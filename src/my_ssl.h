#ifndef MY_SSL_H 
#define MY_SSL_H
#define ITER 10000
#define KEYLEN 32
#define SALTLEN 16

#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>



#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

#include <openssl/dh.h>


#include "color.h"



class my_SSL{
    public:
        static bool get_key_pair(const std::string &name);

        static EVP_PKEY* get_public_key(std::string name);

        static bool generate_key_cer(const std::string& private_key_file, const std::string& certificate_file);


        static SSL_CTX* create_ssl_ctx();

        static void config_ctx(SSL_CTX* ctx, std::string name);

        static EVP_PKEY* pubkey(SSL_CTX* ctx);

        static std::string pubk_toString(EVP_PKEY* pubkey);

        static EVP_PKEY* pubk_fromString(const std::string& pubkey_str);

        static std::string format_key(const std::string& key);
        
        static std::string encrypt(const std::string& plaintext, EVP_PKEY* pubkey);

        static EVP_PKEY* privkey(std::string name);
        
        static std::string decrypt(const std::string& ciphertext, std::string name);
    
        static std::string base64_encode(const std::string& input);

        static std::string base64_decode(const std::string &input);

        static bool rename_file(std::string new_name, std::string old_name);

        static bool compare_evp_pkeys(EVP_PKEY* pkey1, EVP_PKEY* pkey2);

        static std::string re_encrypt(const std::string& input);

        static bool is_base64(const std::string& input);

        static bool gen_aes(const std::string& secret, std::vector<unsigned char>&key);

        static std::string aes_encrypt(std::string plaintext, const unsigned char* key);

        static std::string aes_decrypt(std::string ciphertext, const unsigned char* key);

        static void handleOpenSSLErrors(void);
};

#endif