#pragma once

/*
   SHA-1 in C
   By Steve Reid <steve@edmweb.com>
   100% Public Domain
 */

/* for uint32_t */
#include <stdint.h>

typedef struct
{
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform( uint32_t state[5], const unsigned char buffer[64]);
void SHA1Init(SHA1_CTX * context);
void SHA1Update(SHA1_CTX * context, const unsigned char *data, uint32_t len);
void SHA1Final(unsigned char digest[20], SHA1_CTX * context);
void SHA1(char *hash_out, const char *str, int len);
void SHA1DigestString(const unsigned char digest[20], char digest_string[41]);
void SHA1Undigest(unsigned char digest[20], const char digest_string[41]);
