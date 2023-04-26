/*
Implementation by the Keccak Team, namely, Guido Bertoni, Joan Daemen,
Michaël Peeters, Gilles Van Assche and Ronny Van Keer, 
hereby denoted as "the implementer".

For more information, feedback or questions, please refer to our website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#ifndef FIPS202_32_H
#define FIPS202_32_H

#ifdef ALIGN
#undef ALIGN
#endif

#include <stddef.h>
#include <stdint.h>

#if defined(__GNUC__)
#define ALIGN(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
#define ALIGN(x) __declspec(align(x))
#elif defined(__ARMCC_VERSION)
#define ALIGN(x) __align(x)
#else
#define ALIGN(x)
#endif

#define SHAKE128_RATE 168
#define SHAKE256_RATE 136
#define SHA3_256_RATE 136
#define SHA3_512_RATE 72

typedef unsigned char UINT8;

typedef struct {
  ALIGN(32) unsigned char s[200];
  unsigned int pos;
} keccak_state;


void shake128_init(keccak_state *state);

void shake128_absorb(keccak_state *state, const uint8_t *in, size_t inlen);

void shake128_finalize(keccak_state *state);

void shake128_squeezeblocks(uint8_t *out, size_t nblocks, keccak_state *state);

void shake128_squeeze(uint8_t *out, size_t outlen, keccak_state *state);

void shake128(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);


void shake256_init(keccak_state *state);

void shake256_absorb(keccak_state *state, const uint8_t *in, size_t inlen);

void shake256_finalize(keccak_state *state);

void shake256_squeezeblocks(uint8_t *out, size_t nblocks,  keccak_state *state);

void shake256_squeeze(uint8_t *out, size_t outlen, keccak_state *state);


void shake128(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);

void shake256(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);

void sha3_256(uint8_t h[32], const uint8_t *in, size_t inlen);

void sha3_512(uint8_t h[64], const uint8_t *in, size_t inlen);

#endif
