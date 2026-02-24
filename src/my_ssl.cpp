#include "my_ssl.h"



bool my_SSL::generate_key_cer(const std::string& private_key_file, const std::string& certificate_file) {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    // Generate a new RSA private key
    RSA* rsa = RSA_new();
    BIGNUM* bne = BN_new();
    BN_set_word(bne, RSA_F4);
    RSA_generate_key_ex(rsa, 2048, bne, nullptr);
    BN_free(bne);

    // Create a new X.509 certificate
    X509* x509 = X509_new();
    if (!x509) {
        std::cerr << "Error creating X.509 certificate" << std::endl;
        ERR_print_errors_fp(stderr);
        RSA_free(rsa);
        return false;
    }

    // Set the certificate version to X509v3
    X509_set_version(x509, 2);

    // Set the serial number (can be any unique value)
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

    // Set validity period for 365 days
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 60 * 60 * 24 * 365);

    // Set the subject name (can be any name)
    X509_NAME* name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (const unsigned char*)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (const unsigned char*)"localhost", -1, -1, 0);

    // Set the issuer name (self-signed)
    X509_set_issuer_name(x509, name);

    // Convert RSA to EVP_PKEY
    EVP_PKEY* pkey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pkey, rsa);

    // Set the public key in the certificate
    X509_set_pubkey(x509, pkey);

    // Sign the certificate with the private key
    if (!X509_sign(x509, pkey, EVP_sha256())) {
        std::cerr << "Error signing the certificate" << std::endl;
        ERR_print_errors_fp(stderr);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return false;
    }

    // Write the private key to a file
    FILE* private_key_fp = fopen(private_key_file.c_str(), "w");
    PEM_write_PKCS8PrivateKey(private_key_fp, pkey, nullptr, nullptr, 0, nullptr, nullptr);
    fclose(private_key_fp);

    // Write the certificate to a file
    FILE* certificate_fp = fopen(certificate_file.c_str(), "w");
    PEM_write_X509(certificate_fp, x509);
    fclose(certificate_fp);

    // Free memory
    X509_free(x509);
    EVP_PKEY_free(pkey);

    // Cleanup OpenSSL
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();

    return true;
}

bool my_SSL::get_key_pair(const std::string &name) {
   
    
    SSL_CTX *ctx = create_ssl_ctx();
    config_ctx(ctx, name);

    EVP_PKEY* pkey = SSL_CTX_get0_privatekey(ctx);
    if (pkey) {
        Color::print_green("Private key loaded successfully.");
        PEM_write_PrivateKey(stdout, pkey, nullptr, nullptr, 0, nullptr, nullptr);

        EVP_PKEY* pukey = pubkey(ctx);
        if (pubkey) {
            
                
            PEM_write_PUBKEY(stdout, pukey);
            
            EVP_PKEY_free(pukey);
            
        }
    
    } else {
        Color::print_red( "Error getting private key." );
    }

   
    SSL_CTX_free(ctx);

    return true;
}
EVP_PKEY* my_SSL::get_public_key(std::string name) {
    SSL_CTX* ctx = create_ssl_ctx();
    config_ctx(ctx, name);

    EVP_PKEY* pkey = SSL_CTX_get0_privatekey(ctx);
    if (pkey) {
        Color::print_green("Private key loaded successfully.");

        EVP_PKEY* pukey = pubkey(ctx);
        if (pukey) {
            SSL_CTX_free(ctx); 
            return pukey;
        }
    } else {
        Color::print_red("Error getting private key.");
    }

    SSL_CTX_free(ctx);
    return nullptr;
}

SSL_CTX* my_SSL::create_ssl_ctx() {
    SSL_CTX* ctx;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(TLS_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}
void my_SSL::config_ctx(SSL_CTX* ctx, std::string name) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (name.empty()) return;

    std::string key_filename = name + ".key";
    std::string ctr_filename = name + ".crt";

    // If the certificate does not exist, generate one
    if (access(ctr_filename.c_str(), F_OK) == -1 || access(key_filename.c_str(), F_OK) == -1) { 
        generate_key_cer(key_filename, ctr_filename);
    }

    // Load client certificates and private keys
    if (SSL_CTX_use_certificate_file(ctx, ctr_filename.c_str(), SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, key_filename.c_str(), SSL_FILETYPE_PEM) <= 0) {
       
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
    }
}

EVP_PKEY* my_SSL::pubkey(SSL_CTX* ctx) {

    // Get the X509 certificate from the SSL context
    
    X509* cert = SSL_CTX_get0_certificate(ctx);

    if (!cert) {
        Color::print_red("loading certificate went wrong");
        return nullptr;

    }

    // Extract public key from the certificate
    EVP_PKEY* pukey = X509_get_pubkey(cert);



    return pukey;
}

EVP_PKEY* my_SSL::privkey(std::string name) {
    std::string key_filename = name + ".key"; 

    FILE* fp = fopen(key_filename.c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "Unable to open key file: " << key_filename << std::endl;
        return nullptr;
    }

    EVP_PKEY* pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);

    if (pkey == nullptr) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    return pkey;
}


std::string my_SSL::pubk_toString(EVP_PKEY* pubkey) {
    if (!pubkey) {
        return "";
    }

    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        return "";
    }

   
    if (PEM_write_bio_PUBKEY(bio, pubkey) != 1) {
        BIO_free(bio);
        return "";
    }

    char* buffer;
    long length = BIO_get_mem_data(bio, &buffer);
    std::string result(buffer, length);

    BIO_free(bio);

    return result;
}



EVP_PKEY* my_SSL::pubk_fromString(const std::string& pubkey_str) {
    if (pubkey_str.empty()) {
        std::cerr << "Empty public key string." << std::endl;
        return nullptr;
    }

    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

  
    BIO* bio = BIO_new_mem_buf(pubkey_str.c_str(), -1);

    if (!bio) {
        std::cerr << "Error creating BIO object" << std::endl;
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    
    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);

    if (!pkey) {
        std::cerr << "Error reading public key from string" << std::endl;
        ERR_print_errors_fp(stderr);
    }

  
    BIO_free(bio);

    return pkey;
}
std::string my_SSL:: format_key(const std::string& key){

    
    if (key.empty() || 
        key.find("-----BEGIN PUBLIC KEY-----") != std::string::npos ||
        key.find("-----END PUBLIC KEY-----") != std::string::npos) {
        return key;  // No formatting needed
    }

    
    std::string formatted_key = "-----BEGIN PUBLIC KEY-----\n";

    
    for (size_t i = 0; i < key.length(); i += 64) {
        formatted_key += key.substr(i, 64) + "\n";
    }

   
    formatted_key += "-----END PUBLIC KEY-----\n";

    return formatted_key;
}


std::string my_SSL::encrypt(const std::string& plaintext, EVP_PKEY* pubkey) {
    std::vector<unsigned char> cipher(RSA_size(EVP_PKEY_get0_RSA(pubkey)), 0);
    int cipher_length = RSA_public_encrypt(
        plaintext.size(),
        reinterpret_cast<const unsigned char*>(plaintext.data()),
        cipher.data(),
        EVP_PKEY_get1_RSA(pubkey),
        RSA_PKCS1_OAEP_PADDING);

    if (cipher_length == -1) {
        Color::print_red("could not encrypt");
        ERR_print_errors_fp(stderr);
        abort();
        return {};
    }
     // Convert binary cipher to std::string
    std::string binary_cipher(cipher.begin(), cipher.begin() + cipher_length);

    // Base64 encode
    return base64_encode(binary_cipher);
}



std::string my_SSL::decrypt(const std::string& ciphertext, std::string name) {
   // Base64 decode
    std::string binary_ciphertext = base64_decode(ciphertext);

  
    // RSA decryption
    EVP_PKEY* prikey = privkey(name);
    if (!prikey) return {};
    std::vector<unsigned char> plaintext(RSA_size(EVP_PKEY_get1_RSA(prikey)), 0);
    int plaintext_len = RSA_private_decrypt(
        binary_ciphertext.size(),
        reinterpret_cast<const unsigned char*>(binary_ciphertext.data()),
        plaintext.data(),
        EVP_PKEY_get1_RSA(prikey),
        RSA_PKCS1_OAEP_PADDING);

    if (plaintext_len == -1) {
        ERR_print_errors_fp(stderr);
        abort(); 
        return {};
    }

    return std::string(plaintext.begin(), plaintext.begin() + plaintext_len);
}

std::string my_SSL::base64_encode(const std::string& input) {
    if (input.empty()) {
        return "";
    }

    // Calculate the length of the output string: 4/3 the size of the input, plus padding
    size_t output_length = 4 * ((input.size() + 2) / 3);

    std::vector<unsigned char> encoded_data(output_length + 1); // +1 for null terminator

    int actual_output_length = EVP_EncodeBlock(encoded_data.data(), 
                                               reinterpret_cast<const unsigned char*>(input.data()), 
                                               input.size());

    if (actual_output_length < 0) {
        throw std::runtime_error("Base64 encoding failed.");
    }

    return std::string(reinterpret_cast<char*>(encoded_data.data()));
}

std::string my_SSL::base64_decode(const std::string& input) {
    if (input.empty() || input.size() % 4 != 0) {
        return input;
    }

    size_t output_length = 3 * input.size() / 4;

    std::vector<unsigned char> decoded_data(output_length); // Output length might be too much due to padding, will adjust later.

    int actual_output_length = EVP_DecodeBlock(decoded_data.data(), 
                                               reinterpret_cast<const unsigned char*>(input.data()), 
                                               input.size());

    if (actual_output_length < 0) {
        throw std::runtime_error("Base64 decoding failed.");
    }

    // Adjust length for padding
    if (input.size() >= 2) {
        if (input[input.size() - 1] == '=') actual_output_length--;
        if (input[input.size() - 2] == '=') actual_output_length--;
    }

    return std::string(reinterpret_cast<char*>(decoded_data.data()), actual_output_length);
}

bool my_SSL::rename_file(std::string new_name, std::string old_name) {
    if (new_name.empty() || old_name.empty()) return false;

    std::string old_key_filename = old_name + ".key";
    std::string old_ctr_filename = old_name + ".crt";

    std::string new_key_filename = new_name + ".key";
    std::string new_ctr_filename = new_name + ".crt";

    if (access(old_ctr_filename.c_str(), F_OK) == -1 || access(old_key_filename.c_str(), F_OK) == -1) { 
        return false;
    } 
    if (std::rename(old_key_filename.c_str(), new_key_filename.c_str()) != 0) {
        perror("Error renaming key file");
        return false;
    } else {
        std::printf("Key File renamed successfully\n");
    }
    if (std::rename(old_ctr_filename.c_str(), new_ctr_filename.c_str()) != 0) {
        perror("Error renaming certificate file");
        return false;
    } else {
        std::printf("Certificate File renamed successfully\n");
    }
    return true;
}


DH* generate_dh_parameters(int prime_len) {
    DH* dh = DH_new();
    if (!dh) {
        std::cerr << "Error creating Diffie-Hellman parameters" << std::endl;
        return nullptr;
    }

    if (DH_generate_parameters_ex(dh, prime_len, DH_GENERATOR_2, nullptr) != 1) {
        std::cerr << "Error generating Diffie-Hellman parameters" << std::endl;
        DH_free(dh);
        return nullptr;
    }

    return dh;
}

bool my_SSL::compare_evp_pkeys(EVP_PKEY* pkey1, EVP_PKEY* pkey2) {
    if (pkey1 == nullptr || pkey2 == nullptr) {
        return false; // One or both keys are null, cannot compare.
    }

    // Convert both EVP_PKEYs to public key PEM format for comparison.
    BIO* bio1 = BIO_new(BIO_s_mem());
    BIO* bio2 = BIO_new(BIO_s_mem());

    if (!PEM_write_bio_PUBKEY(bio1, pkey1) || !PEM_write_bio_PUBKEY(bio2, pkey2)) {
        BIO_free(bio1);
        BIO_free(bio2);
        return false; // Failed to write keys to bio, cannot compare.
    }

    // Retrieve the data from BIOs.
    char* keyData1 = nullptr;
    long len1 = BIO_get_mem_data(bio1, &keyData1);
    std::string strKey1(keyData1, len1);

    char* keyData2 = nullptr;
    long len2 = BIO_get_mem_data(bio2, &keyData2);
    std::string strKey2(keyData2, len2);

    // Clean up BIO objects.
    BIO_free(bio1);
    BIO_free(bio2);

    // Compare the PEM strings.
    return strKey1 == strKey2;
}

std::string my_SSL::re_encrypt(const std::string& input) {

    // Decode the input to check if it's valid base64
    std::string decoded = base64_decode(input);
    if (!decoded.empty()) {
        // If it is valid base64, re-encode it
        return base64_encode(decoded);
    }
    
    return input;
}

bool my_SSL::gen_aes(const std::string& secret, std::vector<unsigned char>& key) {
    const unsigned char* secret_bytes = reinterpret_cast<const unsigned char*>(secret.data());
    size_t secret_len = secret.length();

    std::vector<unsigned char> salt(SALTLEN);
    if (RAND_bytes(salt.data(), SALTLEN) != 1) {
        return false; // Failed to generate random salt
    }

    key.resize(KEYLEN); // Prepare the key vector to hold the generated key

    // Generate the key
    int success = PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(secret_bytes), secret_len, salt.data(), SALTLEN, ITER, EVP_sha256(), KEYLEN, key.data());

    return success != 0; // Returns true if key generation succeeded
}


std::string my_SSL::aes_encrypt(std::string plaintext, const unsigned char* key) {
   
     EVP_CIPHER_CTX *ctx;
    std::vector<unsigned char> ciphertext(plaintext.length() + EVP_MAX_BLOCK_LENGTH);
    int len;
    int ciphertext_len;

    if(!(ctx = EVP_CIPHER_CTX_new())) handleOpenSSLErrors();

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL)) // No IV for ECB
        handleOpenSSLErrors();

    if(1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length()))
        handleOpenSSLErrors();
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) handleOpenSSLErrors();
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return std::string(ciphertext.begin(), ciphertext.begin() + ciphertext_len);
}



std::string my_SSL::aes_decrypt(std::string ciphertext, const unsigned char* key) {
   EVP_CIPHER_CTX *ctx;
    std::vector<unsigned char> decrypted(ciphertext.size());
    int len;
    int decrypted_len;

    if(!(ctx = EVP_CIPHER_CTX_new())) handleOpenSSLErrors();

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL)) // No IV for ECB
        handleOpenSSLErrors();

    if(1 != EVP_DecryptUpdate(ctx, decrypted.data(), &len, reinterpret_cast<const unsigned char*>(ciphertext.c_str()), ciphertext.length()))
        handleOpenSSLErrors();
    decrypted_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, decrypted.data() + len, &len)) {
        handleOpenSSLErrors(); // This will print the error and abort; consider a more graceful error handling
        EVP_CIPHER_CTX_free(ctx);
        return ""; // Indicate failure more gracefully in a real application
    }

    decrypted_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Return the decrypted data as a string
    // This assumes that the decrypted data is properly null-terminated. If it's not, you'll need to adjust accordingly.
    return std::string(decrypted.begin(), decrypted.begin() + decrypted_len);
}
void my_SSL:: handleOpenSSLErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}
