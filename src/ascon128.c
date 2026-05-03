#include "ascon128.h"
#include <string.h>

// Endianness handling functions
static inline uint64_t bytes_to_u64(const uint8_t* bytes, size_t n) {
    uint64_t x = 0;
    for (size_t i = 0; i < n; ++i) {
        x |= ((uint64_t)bytes[i]) << (56 - 8 * i);
    }
    return x;
}

static inline void u64_to_bytes(uint8_t* bytes, uint64_t x, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        bytes[i] = (uint8_t)(x >> (56 - 8 * i));
    }
}

// Right rotation
static inline uint64_t rotr(uint64_t x, int n) {
    return (x >> n) | (x << (64 - n));
}

// ASCON Permutation Constants
static const uint8_t RC[12] = {
    0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b
};

// Permutation implementation
void ascon_permutation(ascon_state_t* s, int rounds) {
    int start_round = 12 - rounds;
    
    for (int r = start_round; r < 12; ++r) {
        // 1. Addition of Constants
        s->x[2] ^= RC[r];

        // 2. Substitution Layer (5-bit S-box)
        uint64_t x0 = s->x[0], x1 = s->x[1], x2 = s->x[2], x3 = s->x[3], x4 = s->x[4];
        x0 ^= x4; x4 ^= x3; x2 ^= x1;
        
        uint64_t t0 = x0 ^ (~x1 & x2);
        uint64_t t1 = x1 ^ (~x2 & x3);
        uint64_t t2 = x2 ^ (~x3 & x4);
        uint64_t t3 = x3 ^ (~x4 & x0);
        uint64_t t4 = x4 ^ (~x0 & x1);
        
        t1 ^= t0; t0 ^= t4; t3 ^= t2; t2 = ~t2;
        
        // 3. Linear Diffusion Layer
        s->x[0] = t0 ^ rotr(t0, 19) ^ rotr(t0, 28);
        s->x[1] = t1 ^ rotr(t1, 61) ^ rotr(t1, 39);
        s->x[2] = t2 ^ rotr(t2,  1) ^ rotr(t2,  6);
        s->x[3] = t3 ^ rotr(t3, 10) ^ rotr(t3, 17);
        s->x[4] = t4 ^ rotr(t4,  7) ^ rotr(t4, 41);
    }
}

int crypto_aead_encrypt(
    unsigned char *c, unsigned long long *clen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *nsec,
    const unsigned char *npub,
    const unsigned char *k) 
{
    (void)nsec; // Not used in ASCON
    ascon_state_t s;
    
    // ASCON-128 Initialization
    // IV = 0x80400c0600000000
    s.x[0] = 0x80400c0600000000ULL;
    s.x[1] = bytes_to_u64(k, 8);
    s.x[2] = bytes_to_u64(k + 8, 8);
    s.x[3] = bytes_to_u64(npub, 8);
    s.x[4] = bytes_to_u64(npub + 8, 8);
    
    ascon_permutation(&s, 12);
    s.x[3] ^= bytes_to_u64(k, 8);
    s.x[4] ^= bytes_to_u64(k + 8, 8);
    
    // Process Associated Data
    if (adlen > 0) {
        while (adlen >= 8) {
            s.x[0] ^= bytes_to_u64(ad, 8);
            ascon_permutation(&s, 6);
            ad += 8;
            adlen -= 8;
        }
        // Pad and process remaining AD
        uint8_t padded_ad[8] = {0};
        memcpy(padded_ad, ad, adlen);
        padded_ad[adlen] = 0x80;
        s.x[0] ^= bytes_to_u64(padded_ad, 8);
        ascon_permutation(&s, 6);
    }
    // Domain separation
    s.x[4] ^= 1;
    
    // Process Plaintext
    *clen = 0;
    while (mlen >= 8) {
        s.x[0] ^= bytes_to_u64(m, 8);
        u64_to_bytes(c, s.x[0], 8);
        ascon_permutation(&s, 6);
        m += 8;
        c += 8;
        mlen -= 8;
        *clen += 8;
    }
    
    // Pad and process remaining Plaintext
    uint8_t padded_m[8] = {0};
    memcpy(padded_m, m, mlen);
    padded_m[mlen] = 0x80;
    
    s.x[0] ^= bytes_to_u64(padded_m, 8);
    u64_to_bytes(c, s.x[0], mlen);
    c += mlen;
    *clen += mlen;
    
    // Finalization
    s.x[1] ^= bytes_to_u64(k, 8);
    s.x[2] ^= bytes_to_u64(k + 8, 8);
    ascon_permutation(&s, 12);
    s.x[3] ^= bytes_to_u64(k, 8);
    s.x[4] ^= bytes_to_u64(k + 8, 8);
    
    // Generate tag
    u64_to_bytes(c, s.x[3], 8);
    u64_to_bytes(c + 8, s.x[4], 8);
    *clen += 16;
    
    return 0;
}

int crypto_aead_decrypt(
    unsigned char *m, unsigned long long *mlen,
    unsigned char *nsec,
    const unsigned char *c, unsigned long long clen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *npub,
    const unsigned char *k) 
{
    (void)nsec;
    if (clen < 16) return -1;
    
    unsigned long long m_len = clen - 16;
    ascon_state_t s;
    
    // ASCON-128 Initialization
    s.x[0] = 0x80400c0600000000ULL;
    s.x[1] = bytes_to_u64(k, 8);
    s.x[2] = bytes_to_u64(k + 8, 8);
    s.x[3] = bytes_to_u64(npub, 8);
    s.x[4] = bytes_to_u64(npub + 8, 8);
    
    ascon_permutation(&s, 12);
    s.x[3] ^= bytes_to_u64(k, 8);
    s.x[4] ^= bytes_to_u64(k + 8, 8);
    
    // Process Associated Data
    if (adlen > 0) {
        while (adlen >= 8) {
            s.x[0] ^= bytes_to_u64(ad, 8);
            ascon_permutation(&s, 6);
            ad += 8;
            adlen -= 8;
        }
        uint8_t padded_ad[8] = {0};
        memcpy(padded_ad, ad, adlen);
        padded_ad[adlen] = 0x80;
        s.x[0] ^= bytes_to_u64(padded_ad, 8);
        ascon_permutation(&s, 6);
    }
    // Domain separation
    s.x[4] ^= 1;
    
    // Process Ciphertext
    *mlen = 0;
    while (m_len >= 8) {
        uint64_t cx = bytes_to_u64(c, 8);
        u64_to_bytes(m, s.x[0] ^ cx, 8);
        s.x[0] = cx;
        ascon_permutation(&s, 6);
        m += 8;
        c += 8;
        m_len -= 8;
        *mlen += 8;
    }
    
    // Process remaining Ciphertext
    uint8_t padded_c[8] = {0};
    memcpy(padded_c, c, m_len);
    
    uint64_t cx = bytes_to_u64(padded_c, m_len);
    uint64_t mask = (m_len == 0) ? 0 : (~0ULL) << (64 - 8 * m_len);
    uint64_t mx = (s.x[0] ^ cx) & mask;
    
    u64_to_bytes(m, mx, m_len);
    s.x[0] = (s.x[0] & ~mask) | cx;
    s.x[0] ^= (0x80ULL << (56 - 8 * m_len));
    
    c += m_len;
    *mlen += m_len;
    
    // Finalization
    s.x[1] ^= bytes_to_u64(k, 8);
    s.x[2] ^= bytes_to_u64(k + 8, 8);
    ascon_permutation(&s, 12);
    s.x[3] ^= bytes_to_u64(k, 8);
    s.x[4] ^= bytes_to_u64(k + 8, 8);
    
    // Verify tag
    uint64_t expected_tag_h = s.x[3];
    uint64_t expected_tag_l = s.x[4];
    uint64_t actual_tag_h = bytes_to_u64(c, 8);
    uint64_t actual_tag_l = bytes_to_u64(c + 8, 8);
    
    if ((expected_tag_h != actual_tag_h) || (expected_tag_l != actual_tag_l)) {
        memset(m - *mlen, 0, *mlen); // Zero out plaintext
        return -1; // Authentication failed
    }
    
    return 0; // Success
}
