#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ascon128.h"

void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    printf("ASCON-128 Test Vectors\n");
    printf("----------------------\n");

    unsigned char k[16]    = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    unsigned char npub[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    unsigned char ad[32]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                              16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    unsigned char m[32]    = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                              16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    
    unsigned char c[64];
    unsigned long long clen;

    unsigned char m_dec[64];
    unsigned long long mlen;

    printf("Key              : ");
    for(int i=0; i<16; i++) printf("%02x", k[i]);
    printf("\n");
    
    printf("Nonce            : ");
    for(int i=0; i<16; i++) printf("%02x", npub[i]);
    printf("\n");
    
    printf("Associated Data  : ");
    for(int i=0; i<32; i++) printf("%02x", ad[i]);
    printf("\n");
    
    printf("Plaintext        : ");
    for(int i=0; i<32; i++) printf("%02x", m[i]);
    printf("\n");

    // Encrypt
    crypto_aead_encrypt(c, &clen, m, 32, ad, 32, NULL, npub, k);
    printf("Ciphertext + Tag : ");
    for(unsigned long long i=0; i<clen; i++) printf("%02x", c[i]);
    printf("\n");

    // Decrypt
    int res = crypto_aead_decrypt(m_dec, &mlen, NULL, c, clen, ad, 32, npub, k);
    
    if (res == 0) {
        printf("Decryption successful!\n");
        printf("Decrypted PT     : ");
        for(unsigned long long i=0; i<mlen; i++) printf("%02x", m_dec[i]);
        printf("\n");
    } else {
        printf("Decryption failed! Authentication error.\n");
    }

    return 0;
}
