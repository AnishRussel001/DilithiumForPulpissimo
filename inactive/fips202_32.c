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

#include "fips202_32.h"

extern void keccakf1600(uint32_t *lanes);

static void memsetLokal(unsigned char *buf, unsigned char Const, unsigned int count)
{
    int i;
    for (i = 0; i < count; i++) {
        buf[i] = Const;
    }
}

static void memcpyLokal(unsigned char *dest, const unsigned char *src, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        dest[i] = src[i];
    }
}

/* Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
#define prepareToBitInterleaving(low, high, temp, temp0, temp1) \
        temp0 = (low); \
        temp = (temp0 ^ (temp0 >>  1)) & 0x22222222UL;  temp0 = temp0 ^ temp ^ (temp <<  1); \
        temp = (temp0 ^ (temp0 >>  2)) & 0x0C0C0C0CUL;  temp0 = temp0 ^ temp ^ (temp <<  2); \
        temp = (temp0 ^ (temp0 >>  4)) & 0x00F000F0UL;  temp0 = temp0 ^ temp ^ (temp <<  4); \
        temp = (temp0 ^ (temp0 >>  8)) & 0x0000FF00UL;  temp0 = temp0 ^ temp ^ (temp <<  8); \
        temp1 = (high); \
        temp = (temp1 ^ (temp1 >>  1)) & 0x22222222UL;  temp1 = temp1 ^ temp ^ (temp <<  1); \
        temp = (temp1 ^ (temp1 >>  2)) & 0x0C0C0C0CUL;  temp1 = temp1 ^ temp ^ (temp <<  2); \
        temp = (temp1 ^ (temp1 >>  4)) & 0x00F000F0UL;  temp1 = temp1 ^ temp ^ (temp <<  4); \
        temp = (temp1 ^ (temp1 >>  8)) & 0x0000FF00UL;  temp1 = temp1 ^ temp ^ (temp <<  8);

#define toBitInterleavingAndXOR(low, high, even, odd, temp, temp0, temp1) \
        prepareToBitInterleaving(low, high, temp, temp0, temp1) \
        even ^= (temp0 & 0x0000FFFF) | (temp1 << 16); \
        odd ^= (temp0 >> 16) | (temp1 & 0xFFFF0000);

/* Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
#define prepareFromBitInterleaving(even, odd, temp, temp0, temp1) \
        temp0 = (even); \
        temp1 = (odd); \
        temp = (temp0 & 0x0000FFFF) | (temp1 << 16); \
        temp1 = (temp0 >> 16) | (temp1 & 0xFFFF0000); \
        temp0 = temp; \
        temp = (temp0 ^ (temp0 >>  8)) & 0x0000FF00UL;  temp0 = temp0 ^ temp ^ (temp <<  8); \
        temp = (temp0 ^ (temp0 >>  4)) & 0x00F000F0UL;  temp0 = temp0 ^ temp ^ (temp <<  4); \
        temp = (temp0 ^ (temp0 >>  2)) & 0x0C0C0C0CUL;  temp0 = temp0 ^ temp ^ (temp <<  2); \
        temp = (temp0 ^ (temp0 >>  1)) & 0x22222222UL;  temp0 = temp0 ^ temp ^ (temp <<  1); \
        temp = (temp1 ^ (temp1 >>  8)) & 0x0000FF00UL;  temp1 = temp1 ^ temp ^ (temp <<  8); \
        temp = (temp1 ^ (temp1 >>  4)) & 0x00F000F0UL;  temp1 = temp1 ^ temp ^ (temp <<  4); \
        temp = (temp1 ^ (temp1 >>  2)) & 0x0C0C0C0CUL;  temp1 = temp1 ^ temp ^ (temp <<  2); \
        temp = (temp1 ^ (temp1 >>  1)) & 0x22222222UL;  temp1 = temp1 ^ temp ^ (temp <<  1);

#define fromBitInterleaving(even, odd, low, high, temp, temp0, temp1) \
        prepareFromBitInterleaving(even, odd, temp, temp0, temp1) \
        low = temp0; \
        high = temp1;

static void KeccakP1600_AddByte(void *state, unsigned char byte, unsigned int offset)
{
    unsigned int lanePosition = offset/8;
    unsigned int offsetInLane = offset%8;
    uint32_t low, high;
    uint32_t temp, temp0, temp1;
    uint32_t *stateAsHalfLanes = (uint32_t*)state;

    if (offsetInLane < 4) {
        low = (uint32_t)byte << (offsetInLane*8);
        high = 0;
    }
    else {
        low = 0;
        high = (uint32_t)byte << ((offsetInLane-4)*8);
    }
    toBitInterleavingAndXOR(low, high, stateAsHalfLanes[lanePosition*2+0], stateAsHalfLanes[lanePosition*2+1], temp, temp0, temp1);
}

static void KeccakP1600_AddBytesInLane(void *state, unsigned int lanePosition, const unsigned char *data, unsigned int offset, unsigned int length)
{
    UINT8 laneAsBytes[8];
    uint32_t low, high;
    uint32_t temp, temp0, temp1;
    uint32_t *stateAsHalfLanes = (uint32_t*)state;

    memsetLokal(laneAsBytes, 0, 8);
    memcpyLokal(laneAsBytes+offset, data, length);
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    low = *((uint32_t*)(laneAsBytes+0));
    high = *((uint32_t*)(laneAsBytes+4));
#else
    low = laneAsBytes[0]
        | ((uint32_t)(laneAsBytes[1]) << 8)
        | ((uint32_t)(laneAsBytes[2]) << 16)
        | ((uint32_t)(laneAsBytes[3]) << 24);
    high = laneAsBytes[4]
        | ((uint32_t)(laneAsBytes[5]) << 8)
        | ((uint32_t)(laneAsBytes[6]) << 16)
        | ((uint32_t)(laneAsBytes[7]) << 24);
#endif
    toBitInterleavingAndXOR(low, high, stateAsHalfLanes[lanePosition*2+0], stateAsHalfLanes[lanePosition*2+1], temp, temp0, temp1);
}

static void KeccakP1600_AddLanes(void *state, const unsigned char *data, unsigned int laneCount, unsigned int offset)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    const uint32_t * pI = (const uint32_t *)data;
    uint32_t * pS = (uint32_t*)state;
    uint32_t t, x0, x1;
    int i;
    if (offset != 0) {
        pS += (offset/4);
    }
    for (i = laneCount-1; i >= 0; --i) {
#ifdef NO_MISALIGNED_ACCESSES
        uint32_t low;
        uint32_t high;
        memcpyLokal(&low, pI++, 4);
        memcpyLokal(&high, pI++, 4);
        toBitInterleavingAndXOR(low, high, *(pS++), *(pS++), t, x0, x1);
#else
        toBitInterleavingAndXOR(*(pI++), *(pI++), *(pS++), *(pS++), t, x0, x1)
#endif
    }
#else
    unsigned int lanePosition;
    for(lanePosition=0; lanePosition<laneCount; lanePosition++) {
        UINT8 laneAsBytes[8];
        memcpyLokal(laneAsBytes, data+lanePosition*8, 8);
        uint32_t low = laneAsBytes[0]
            | ((uint32_t)(laneAsBytes[1]) << 8)
            | ((uint32_t)(laneAsBytes[2]) << 16)
            | ((uint32_t)(laneAsBytes[3]) << 24);
        uint32_t high = laneAsBytes[4]
            | ((uint32_t)(laneAsBytes[5]) << 8)
            | ((uint32_t)(laneAsBytes[6]) << 16)
            | ((uint32_t)(laneAsBytes[7]) << 24);
        uint32_t even, odd, temp, temp0, temp1;
        uint32_t *stateAsHalfLanes = (uint32_t*)state;
        toBitInterleavingAndXOR(low, high, stateAsHalfLanes[lanePosition*2+0], stateAsHalfLanes[lanePosition*2+1], temp, temp0, temp1);
    }
#endif
}

static void KeccakP1600_AddBytes(void *state, const unsigned char *data, unsigned int offset, unsigned int length)
{
    unsigned int logOffset = offset / 8;
    if ((offset) == 0) {
        KeccakP1600_AddLanes(state, data, (length)/8, 0);
        KeccakP1600_AddBytesInLane(state, (length)/8, (data)+((length)/8)*8, 0, (length)%8);
    }
    else {
        KeccakP1600_AddLanes(state, data, (length)/8, offset);
        /* KeccakP1600_AddBytesInLane offset = byteoffset in a lane */
        KeccakP1600_AddBytesInLane(state, (length)/8, (data)+((length)/8)*8, 0, (length)%8);
    }
}

static void KeccakP1600_ExtractBytesInLane(const void *state, unsigned int lanePosition, unsigned char *data, unsigned int offset, unsigned int length)
{
    uint32_t *stateAsHalfLanes = (uint32_t*)state;
    uint32_t low, high, temp, temp0, temp1;
    UINT8 laneAsBytes[8];

    fromBitInterleaving(stateAsHalfLanes[lanePosition*2], stateAsHalfLanes[lanePosition*2+1], low, high, temp, temp0, temp1);
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    *((uint32_t*)(laneAsBytes+0)) = low;
    *((uint32_t*)(laneAsBytes+4)) = high;
#else
    laneAsBytes[0] = low & 0xFF;
    laneAsBytes[1] = (low >> 8) & 0xFF;
    laneAsBytes[2] = (low >> 16) & 0xFF;
    laneAsBytes[3] = (low >> 24) & 0xFF;
    laneAsBytes[4] = high & 0xFF;
    laneAsBytes[5] = (high >> 8) & 0xFF;
    laneAsBytes[6] = (high >> 16) & 0xFF;
    laneAsBytes[7] = (high >> 24) & 0xFF;
#endif
    memcpyLokal(data, laneAsBytes+offset, length);
}

static void KeccakP1600_ExtractLanes(const void *state, unsigned char *data, unsigned int laneCount, unsigned int offset)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    uint32_t * pI = (uint32_t *)data;
    const uint32_t * pS = ( const uint32_t *)state;
    uint32_t t, x0, x1;
    int i;

    if (offset) {
        pS += (offset/4);
    }
    for (i = laneCount-1; i >= 0; --i) {
#ifdef NO_MISALIGNED_ACCESSES
        uint32_t low;
        uint32_t high;
        fromBitInterleaving(*(pS++), *(pS++), low, high, t, x0, x1);
        memcpyLokal(pI++, &low, 4);
        memcpyLokal(pI++, &high, 4);
#else
        fromBitInterleaving(*(pS++), *(pS++), *(pI++), *(pI++), t, x0, x1)
#endif
    }
#else
    unsigned int lanePosition;
    for(lanePosition=0; lanePosition<laneCount; lanePosition++) {
        uint32_t *stateAsHalfLanes = (uint32_t*)state;
        uint32_t low, high, temp, temp0, temp1;
        fromBitInterleaving(stateAsHalfLanes[lanePosition*2], stateAsHalfLanes[lanePosition*2+1], low, high, temp, temp0, temp1);
        UINT8 laneAsBytes[8];
        laneAsBytes[0] = low & 0xFF;
        laneAsBytes[1] = (low >> 8) & 0xFF;
        laneAsBytes[2] = (low >> 16) & 0xFF;
        laneAsBytes[3] = (low >> 24) & 0xFF;
        laneAsBytes[4] = high & 0xFF;
        laneAsBytes[5] = (high >> 8) & 0xFF;
        laneAsBytes[6] = (high >> 16) & 0xFF;
        laneAsBytes[7] = (high >> 24) & 0xFF;
        memcpyLokal(data+lanePosition*8, laneAsBytes, 8);
    }
#endif
}

static void KeccakP1600_ExtractBytes(const void *state, unsigned char *data, unsigned int offset, unsigned int length)
{
    if ((offset) == 0) {
        KeccakP1600_ExtractLanes(state, data, (length)/8, 0);
        KeccakP1600_ExtractBytesInLane(state, (length)/8, (data)+((length)/8)*8, 0, (length)%8);
    }
    else {
        KeccakP1600_ExtractLanes(state, data, (length)/8, offset);
        KeccakP1600_ExtractBytesInLane(state, (length)/8, (data)+((length)/8)*8, 0, (length)%8);   
    }
}

/* Simple keccak init */
static void keccak_init(keccak_state *state)
{
  unsigned int i;
  for(i=0;i<200;i++)
    state->s[i] = 0;
  state->pos = 0;
}

/*
 * incremental keccak absorb 
 * Author: Anish Rasal Raj
 */
static unsigned int keccak_absorb_inc(uint32_t s[50],
                                  unsigned int r,
                                  unsigned int pos,
                                  const UINT8 *m,
                                  size_t mlen)
{
    int i,j;

    i = pos & 7;
    if(i) {
        /* Calculate available bytes */
        if ((8-i) > mlen)
            j = mlen;
        else
            j = 8-i;

        KeccakP1600_AddBytesInLane(s, pos/8, m, i, j);
        
        m+=(j);
        mlen-=(j);
        pos+=(j);
    }

    if(pos && mlen >= r-pos) {
        KeccakP1600_AddBytes(s, m, pos, (r-pos));
        m += r-pos;
        mlen -= r-pos;
        pos = 0;
        keccakf1600(s);
    }

    while(mlen >= r) {
        KeccakP1600_AddBytes(s, m, pos, r);
        m += r;
        mlen -= r;
        keccakf1600(s);
        pos = 0;
    }

    if(mlen/8 > 0) {
        i = mlen/8 * 8;
        KeccakP1600_AddBytes(s, m, pos, i);
        pos += i;
        m += i;
        mlen -= i;;
    }

    if (mlen) {
        KeccakP1600_AddBytesInLane(s, pos/8, m, 0, mlen);
        pos += mlen;
    }

    return pos;
}

/* Simple kecack finalize*/
static void keccak_finalize(uint32_t s[50], unsigned int r, unsigned int pos, UINT8 p)
{
    /* Finally, absorb the suffix */
    /* Last few bits, whose delimiter coincides with first bit of padding */
    KeccakP1600_AddByte(s, p, pos);
    /* If the first bit of padding is at position rate-1, we need a whole new block for the second bit of padding */
    if ((p >= 0x80) && (pos == (r-1)))
        keccakf1600((uint32_t*)s);
    /* Second bit of padding */
    KeccakP1600_AddByte(s, 0x80, r-1);
    keccakf1600((uint32_t*)s);
}

static void keccak_squeezeblocks(UINT8 *out,
                                 size_t nblocks,
                                 uint32_t s[50],
                                 unsigned int r)
{
    while(nblocks > 0) {
        KeccakP1600_ExtractBytes(s, out, 0, r);
        keccakf1600((uint32_t*)s);
        out += r;
        nblocks--;
    }
}

/*
 * Incremental squeeze
 * Author: Anish Rasal Raj
 */
static unsigned int keccak_squeeze_inc(UINT8 *out,
                                   size_t outlen,
                                   uint32_t s[50],
                                   unsigned int r,
                                   unsigned int pos)
{
    unsigned int i,j;

    i = pos & 7;
    if(i) {
        /* Calculate available bytes */
        if ((8-i) > outlen)
            j = outlen;
        else
            j = 8-i;
        KeccakP1600_ExtractBytesInLane(s, pos/8, out, i, j);
        pos += (j);
        outlen -= (j);
        out += (j);
    }

    if(pos && outlen >= r-pos) {
        KeccakP1600_ExtractBytes(s, out, pos, (r-pos));
        keccakf1600(s);
        out += r-pos;
        outlen -= r-pos;
        pos = 0;
    }

    while(outlen >= r) {
        KeccakP1600_ExtractBytes(s, out, pos, r);
        keccakf1600(s);
        out += r;
        outlen -= r;
        pos = 0;
    }

    if (outlen/8 > 0) {
        i = (outlen/8) * 8;
        KeccakP1600_ExtractBytes(s, out, pos, i);
        out += i;
        outlen -= i;
        pos += i;
    }

    if (outlen) {
        KeccakP1600_ExtractBytesInLane(s, pos/8, out, 0, outlen);
        pos += outlen;
        outlen = 0;
    }

    return pos;
}

/*************************************************
* Name:        shake128_init
*
* Description: Initilizes Keccak state for use as SHAKE128 XOF
*
* Arguments:   - keccak_state *state: pointer to (uninitialized)
*                                     Keccak state
**************************************************/
void shake128_init(keccak_state *state)
{
    keccak_init(state);
}

/*************************************************
* Name:        shake128_absorb
*
* Description: Absorb step of the SHAKE128 XOF; incremental.
*
* Arguments:   - keccak_state *state: pointer to (initialized) output
*                                     Keccak state
*              - const uint8_t *in:   pointer to input to be absorbed into s
*              - size_t inlen:        length of input in bytes
**************************************************/
void shake128_absorb(keccak_state *state, const unsigned char *in, size_t inlen)
{
    state->pos = keccak_absorb_inc((uint32_t*)state->s, SHAKE128_RATE, state->pos, in, inlen);
}

/*************************************************
* Name:        shake128_finalize
*
* Description: Finalize absorb step of the SHAKE128 XOF.
*
* Arguments:   - keccak_state *state: pointer to Keccak state
**************************************************/
void shake128_finalize(keccak_state *state)
{
    keccak_finalize((uint32_t*)state->s, SHAKE128_RATE, state->pos, 0x1F);
    state->pos = 0;
}

/*************************************************
* Name:        shake128_squeezeblocks
*
* Description: Squeeze step of SHAKE128 XOF. Squeezes full blocks of
*              SHAKE128_RATE bytes each. Can be called multiple times
*              to keep squeezing. Assumes zero bytes of current block
*              have already been squeezed (state->pos = 0).
*
* Arguments:   - uint8_t *out:    pointer to output blocks
*              - size_t nblocks:  number of blocks to be squeezed
*                                 (written to output)
*              - keccak_state *s: pointer to input/output Keccak state
**************************************************/
void shake128_squeezeblocks(unsigned char *out, size_t nblocks, keccak_state *state)
{
    keccak_squeezeblocks(out, nblocks, (uint32_t*)state->s, SHAKE128_RATE);
}

/*************************************************
* Name:        shake128_squeeze
*
* Description: Squeeze step of SHAKE128 XOF. Squeezes arbitraily many
*              bytes. Can be called multiple times to keep squeezing.
*
* Arguments:   - uint8_t *out:    pointer to output blocks
*              - size_t outlen :  number of bytes to be squeezed
*                                 (written to output)
*              - keccak_state *s: pointer to input/output Keccak state
**************************************************/
void shake128_squeeze(unsigned char *out, size_t outlen, keccak_state *state)
{
    state->pos = keccak_squeeze_inc(out, outlen, (uint32_t*)state->s, SHAKE128_RATE, state->pos);
}

/*************************************************
* Name:        shake128
*
* Description: SHAKE128 XOF with non-incremental API
*
* Arguments:   - uint8_t *out:      pointer to output
*              - size_t outlen:     requested output length in bytes
*              - const uint8_t *in: pointer to input
*              - size_t inlen:      length of input in bytes
**************************************************/
void shake128(UINT8 *out, size_t outlen, const UINT8 *in, size_t inlen) {
    keccak_state state;

    shake128_init(&state);
    shake128_absorb(&state, in, inlen);
    shake128_finalize(&state);
    shake128_squeeze(out, outlen, &state);
}

/*************************************************
* Name:        shake256_init
*
* Description: Initilizes Keccak state for use as SHAKE256 XOF
*
* Arguments:   - keccak_state *state: pointer to (uninitialized)
*                                     Keccak state
**************************************************/
void shake256_init(keccak_state *state)
{
    keccak_init(state);
}

/*************************************************
* Name:        shake256_absorb
*
* Description: Absorb step of the SHAKE256 XOF; incremental.
*
* Arguments:   - keccak_state *state: pointer to (initialized) output
*                                     Keccak state
*              - const uint8_t *in:   pointer to input to be absorbed into s
*              - size_t inlen:        length of input in bytes
**************************************************/
void shake256_absorb(keccak_state *state, const unsigned char *in, size_t inlen)
{
    state->pos = keccak_absorb_inc((uint32_t*)state->s, SHAKE256_RATE, state->pos, in, inlen);
}

/*************************************************
* Name:        shake256_finalize
*
* Description: Finalize absorb step of the SHAKE256 XOF.
*
* Arguments:   - keccak_state *state: pointer to Keccak state
**************************************************/
void shake256_finalize(keccak_state *state)
{
    keccak_finalize((uint32_t*)state->s, SHAKE256_RATE, state->pos, 0x1F);
    state->pos = 0;
}

/*************************************************
* Name:        shake256_squeezeblocks
*
* Description: Squeeze step of SHAKE256 XOF. Squeezes full blocks of
*              SHAKE256_RATE bytes each. Can be called multiple times
*              to keep squeezing. Assumes zero bytes of current block
*              have already been squeezed (state->pos = 0).
*
* Arguments:   - uint8_t *out:    pointer to output blocks
*              - size_t nblocks:  number of blocks to be squeezed
*                                 (written to output)
*              - keccak_state *s: pointer to input/output Keccak state
**************************************************/
void shake256_squeezeblocks(unsigned char *out, size_t nblocks, keccak_state *state)
{
    keccak_squeezeblocks(out, nblocks, (uint32_t*)state->s, SHAKE256_RATE);
}

/*************************************************
* Name:        shake256_squeeze
*
* Description: Squeeze step of SHAKE256 XOF. Squeezes arbitraily many
*              bytes. Can be called multiple times to keep squeezing.
*
* Arguments:   - uint8_t *out:    pointer to output blocks
*              - size_t outlen :  number of bytes to be squeezed
*                                 (written to output)
*              - keccak_state *s: pointer to input/output Keccak state
**************************************************/
void shake256_squeeze(unsigned char *out, size_t outlen, keccak_state *state)
{
    state->pos = keccak_squeeze_inc(out, outlen, (uint32_t*)state->s, SHAKE256_RATE, state->pos);
}

/*************************************************
* Name:        shake256
*
* Description: SHAKE256 XOF with non-incremental API
*
* Arguments:   - uint8_t *out:      pointer to output
*              - size_t outlen:     requested output length in bytes
*              - const uint8_t *in: pointer to input
*              - size_t inlen:      length of input in bytes
**************************************************/
void shake256(unsigned char *out, size_t outlen, const unsigned char *in, size_t inlen)
{
  keccak_state state;

  shake256_init(&state);
  shake256_absorb(&state, in, inlen);
  shake256_finalize(&state);
  shake256_squeeze(out, outlen, &state);
}

/*************************************************
* Name:        sha3_256
*
* Description: SHA3-256 with non-incremental API
*
* Arguments:   - uint8_t *h:        pointer to output (32 bytes)
*              - const uint8_t *in: pointer to input
*              - size_t inlen:      length of input in bytes
**************************************************/
void sha3_256(unsigned char h[32], const unsigned char *in, size_t inlen)
{
  unsigned char s[200] = {0};
  unsigned int pos;

  pos = keccak_absorb_inc((uint32_t*)s, SHA3_256_RATE, 0, in, inlen);
  keccak_finalize((uint32_t*)s, SHA3_256_RATE, pos, 0x06);
  keccak_squeeze_inc(h, 32, (uint32_t*)s, SHA3_256_RATE, 0);
}

/*************************************************
* Name:        sha3_512
*
* Description: SHA3-512 with non-incremental API
*
* Arguments:   - uint8_t *h:        pointer to output (64 bytes)
*              - const uint8_t *in: pointer to input
*              - size_t inlen:      length of input in bytes
**************************************************/
void sha3_512(unsigned char h[64], const unsigned char *in, size_t inlen)
{
  unsigned char s[200] = {0};
  unsigned int pos;

  pos = keccak_absorb_inc((uint32_t*)s, SHA3_512_RATE, 0, in, inlen);
  keccak_finalize((uint32_t*)s, SHA3_512_RATE, pos, 0x06);
  keccak_squeeze_inc(h, 64, (uint32_t*)s, SHA3_512_RATE, 0);
}
