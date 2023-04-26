#include <stdint.h>
#include "params.h"
#include "symmetric.h"
#include "fips202.h"

#ifndef HWX
void dilithium_shake128_stream_init(keccak_state *state,
                                    const uint8_t seed[SEEDBYTES],
                                    uint16_t nonce)
{
  shake128_init(state);
  shake128_absorb(state, seed, SEEDBYTES);
  stream128_nonce(nonce, state);
}

void dilithium_shake256_stream_init(keccak_state *state,
                                    const uint8_t seed[CRHBYTES],
                                    uint16_t nonce)
{
  shake256_init(state);
  shake256_absorb(state, seed, CRHBYTES);
  stream256_nonce(nonce, state); 
}

#else
void dilithium_shake128_stream_init(const uint8_t seed[SEEDBYTES],
                                    uint16_t nonce)
{
  shake128_init();
  shake128_absorb(seed, SEEDBYTES);
  stream128_nonce(nonce);
}

void dilithium_shake256_stream_init(const uint8_t seed[CRHBYTES],
                                    uint16_t nonce)
{
  shake256_init();
  shake256_absorb(seed, CRHBYTES);
  stream256_nonce(nonce); 
}
#endif
