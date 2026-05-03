#ifndef ASCON128_H
#define ASCON128_H

#include <stdint.h>
#include <stddef.h>

#define ASCON_128_KEYBYTES 16
#define ASCON_128_NPUBBYTES 16
#define ASCON_128_ABYTES 16

// 320-bit state (5 x 64-bit words)
typedef struct {
    uint64_t x[5];
} ascon_state_t;

// Function prototypes
void ascon_permutation(ascon_state_t* s, int rounds);

// Encrypts plaintext to ciphertext and computes authentication tag
int crypto_aead_encrypt(
    unsigned char *c, unsigned long long *clen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *nsec,
    const unsigned char *npub,
    const unsigned char *k
);

// Decrypts ciphertext to plaintext and verifies authentication tag
int crypto_aead_decrypt(
    unsigned char *m, unsigned long long *mlen,
    unsigned char *nsec,
    const unsigned char *c, unsigned long long clen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *npub,
    const unsigned char *k
);

#endif // ASCON128_H
