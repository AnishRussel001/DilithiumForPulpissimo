#ifndef FIPS202_32_CORE
#define FIPS202_32_CORE

// typedefs
typedef unsigned int        UINT32;

// Parameters
#define cKeccakR_SizeInBytes    136
#define cKeccakNumberOfRounds   24
#define cKeccakLaneSizeInBytes 8
#define cKeccakLaneSizeInBits   (cKeccakLaneSizeInBytes * 8)

#define ROL32(a, offset) ((((UINT32)a) << (offset)) ^ (((UINT32)a) >> (32-(offset))))

// Functions
void keccakf1600(uint32_t * state);

#endif
