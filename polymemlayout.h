#ifndef POLYMEMLAYOUT_H
#define POLYMEMLAYOUT_H

#include     "params.h"

/**
  Hardcoded Memory map of Polynomial matrix and polynomial vectors
*/

#define BUF_MEM                0x28

#define POLY_SIZE              0x400
#define POLYVECK_SIZE          (POLY_SIZE * K)
#define POLYVECL_SIZE          (POLY_SIZE * L)

#define DATA_BASE_ADDR          0x1c040000
#define ABS_DATA_END            0x1c080000
#define SOFT_DATA_END           0x1c040000

#define MATRIX_BASE_ADDR      DATA_BASE_ADDR
#define MATRIX_SIZE           (POLYVECL_SIZE * K)

#define S1_BASE_ADDR          (MATRIX_BASE_ADDR + MATRIX_SIZE)
#define S1_SIZE               POLYVECL_SIZE

#define S1HAT_BASE_ADDR       (S1_BASE_ADDR + S1_SIZE)
#define S1HAT_SIZE            POLYVECL_SIZE

#define S2_BASE_ADDR          (S1HAT_BASE_ADDR + S1HAT_SIZE)
#define S2_SIZE               POLYVECK_SIZE

#define T1_BASE_ADDR          (S2_BASE_ADDR + S2_SIZE)
#define T1_SIZE               POLYVECK_SIZE

#define T0_BASE_ADDR          (T1_BASE_ADDR + T1_SIZE)
#define T0_SIZE               (POLYVECK_SIZE)

#define Z_BASE_ADDR           (T0_BASE_ADDR + T0_SIZE)
#define Z_SIZE                POLYVECL_SIZE

#define W1_BASE_ADDR          (Z_BASE_ADDR + Z_SIZE)
#define W1_SIZE               POLYVECK_SIZE

#define W0_BASE_ADDR          (W1_BASE_ADDR + W1_SIZE)
#define W0_SIZE               POLYVECK_SIZE

#define H_BASE_ADDR           (W0_BASE_ADDR + W0_SIZE)
#define H_SIZE                POLYVECK_SIZE

#define CP_BASE_ADDR          (H_BASE_ADDR + H_SIZE)
#define CP_SIZE               POLY_SIZE

// Memory layout of components used by the HW accelerators

#define KECCAK_STATE_BASE_ADDR  (CP_BASE_ADDR + CP_SIZE)
#define KECCAK_STATE_SIZE       (50*4)

// memory layout of sk, pk and sm
#define SK_BASE_ADDR            (KECCAK_STATE_BASE_ADDR + KECCAK_STATE_SIZE)
#define SK_SIZE                 (CRYPTO_SECRETKEYBYTES)

#define PK_BASE_ADDR            (SK_BASE_ADDR + SK_SIZE)
#define PK_SIZE                 CRYPTO_PUBLICKEYBYTES

#define SM_BASE_ADDR            (PK_BASE_ADDR+PK_SIZE)
#if DILITHIUM_MODE == 2
#define SM_SIZE                 (MLEN + CRYPTO_BYTES)
#elif DILITHIUM_MODE == 3
#define SM_SIZE                 (MLEN + 3296)
#elif DILITHIUM_MODE == 5
#define SM_SIZE                 (MLEN + CRYPTO_BYTES + 1)     // one additional byte for memory alignment
#endif

#define M1_BASE_ADDR            (SM_BASE_ADDR + SM_SIZE)
#if DILITHIUM_MODE == 2
#define M1_SIZE                 (MLEN + CRYPTO_BYTES)
#elif DILITHIUM_MODE == 3
#define M1_SIZE                 (MLEN + 3296)
#elif DILITHIUM_MODE == 5
#define M1_SIZE                 (MLEN + CRYPTO_BYTES + 1)     // one additional byte for memory alignment
#endif


#define SEEDBUF_ADDR            (M1_BASE_ADDR + M1_SIZE)
#define SEEDBUF_SIZE            (2*SEEDBYTES + 3*CRHBYTES)

#define M_BASE_ADDR             (SEEDBUF_ADDR + SEEDBUF_SIZE)
#define M_SIZE                  (MLEN)

#define KEYGEN_SEED_ADDR        (M_BASE_ADDR + M_SIZE)
#define KEYGEN_SIZE             (3*SEEDBYTES)

#define KEYGEN_TR_ADDR          (KEYGEN_SEED_ADDR + KEYGEN_SIZE)
#define KEYGEN_TR_SIZE          (CRHBYTES)

#define POLY_BUF_ADDR           (KEYGEN_TR_ADDR + KEYGEN_TR_SIZE)
#define POLY_BUF_SIZE           (1024)      // assign one kb

#define TWD_BASE                (POLY_BUF_ADDR + POLY_BUF_SIZE)
#define TWD_SIZE                (POLY_SIZE)

#define SIGN_BUF_BASE           (TWD_BASE + TWD_SIZE)
#define SIGN_BUF_SIZE           (K*POLYW1_PACKEDBYTES + (3*SEEDBYTES) + CRHBYTES)

#define TMP_POLY_BASE           (SIGN_BUF_BASE + SIGN_BUF_SIZE)
#define TMP_POLY_SIZE           POLY_SIZE

#define VAR_HARD_DATA_END       (TMP_POLY_BASE + TMP_POLY_SIZE)


#endif