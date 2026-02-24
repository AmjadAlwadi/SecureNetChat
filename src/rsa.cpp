
#include "rsa.h"
void RSA::keygen() {
    p = generate_prime();
    q = generate_prime();

    n = p * q;
    phi = (p - 1) * (q - 1);
    e = generate_coprime(phi);
    d = mod_inverse(e, phi);
}

long long RSA::generate_prime() {
    long long num;
    while(true) {
        num = rand() % 200 + 100; //prime number between 100 and 300
        if (is_prime(num)) break;
    }
    return num;
}

bool RSA::is_prime(long long num) {
    //works for small numbers 
    for (int i = 2; i < num; i++) {
        if (num % i == 0) return false;
    }
    return true;
}

long long RSA::generate_coprime(long long phi) {
    long long e = 2;
    while (e < phi) {
        if (gcd(e, phi) == 1) {
            return e;
        }
        e++;
    }
    return -1; // Should not happen
}

long long RSA::mod_inverse(long long e, long long phi) {
    long long d = 2;
    const int max_iterations = 100000;  // Set a maximum limit for iterations

    for (int i = 0; i < max_iterations; ++i) {
        if (gcd(d * e, phi) == 1) {
            return d;
        }
        d++;
    }

    std::cerr << "Error: Modular inverse not found within the maximum limit.\n";
  
}


long long RSA::mod_pow(long long base, long long exponent, long long modulus) {
    base = base % modulus; // Reduce the base modulo modulus

    double result = std::pow(static_cast<double>(base), static_cast<double>(exponent));
    long long result_integer = static_cast<long long>(std::fmod(result, static_cast<double>(modulus)));

    return result_integer;
}
long long RSA::encrypt(long long message) const {
    return mod_pow(message, e, n);
}

long long RSA::decrypt(long long ciphertext) const {
    return mod_pow(ciphertext, d, n);
}