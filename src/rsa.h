#ifndef RSA_H
#define RSA_H

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>


class RSA {
public:
    long long p, q;
    long long n, phi, e, d;

    long long generate_coprime(long long phi);
    long long extended_gcd(long long a, long long b, long long& x, long long& y);
    long long mod_inverse(long long e, long long phi);
    long long gcd(long long a, long long b);
    long long generate_prime();
    bool is_prime(long long num);

    RSA() = default; 
    void keygen();
    long long encrypt(long long message);
    long long decrypt(long long ciphertext);
};

#endif // RSA_H
