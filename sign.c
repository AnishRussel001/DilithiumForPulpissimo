#include <stdint.h>
#include <stdio.h>
#include "params.h"
#include "sign.h"
#include "packing.h"
#include "polyvec.h"
//#include "poly.h"
#include "randombytes.h"
#include "symmetric.h"
#include "fips202.h"

#include <stdlib.h>

#include "polymemlayout.h"

#ifndef X86
#include "pulp.h"
#endif

#ifdef X86
// globals
polyvecl g_mat[K];
polyvecl g_s1, g_s1hat;
polyveck g_s2, g_t1, g_t0;
polyvecl g_y, g_z;
polyveck g_t0, g_w1, g_w0, g_h;
poly g_cp;
#endif

#if 0
/* stack usage optimization */
static void expand_mat_elem(poly *mat_elem, const unsigned char rho[SEEDBYTES], size_t k_idx, size_t l_idx) {
    poly_uniform(mat_elem, rho, (uint16_t)((k_idx << 8) + l_idx));
}

int crypto_sign_keypair(uint8_t *pk, uint8_t *sk)
{
  unsigned char seedbuf[3 * SEEDBYTES];
    unsigned char tr[CRHBYTES];
    const unsigned char *rho, *rhoprime, *key;
    uint16_t nonce = 0;
    polyvecl s1;

    /* Expand 32 bytes of randomness into rho, rhoprime and key */
    randombytes(seedbuf, 3 * SEEDBYTES);

    rho = seedbuf;
    rhoprime = seedbuf + SEEDBYTES;
    key = seedbuf + 2 * SEEDBYTES;
    pack_sk_rho(sk, rho);
    pack_sk_key(sk, key);
    pack_pk_rho(pk, rho);

    /* Sample short vector s1 and immediately store its time-domain version */
    for (size_t l_idx = 0; l_idx < L; ++l_idx) {
        poly_uniform_eta(&s1.vec[l_idx], rhoprime, nonce++);
        pack_sk_s1(sk, &s1.vec[l_idx], l_idx);
        /* Move s1 to NTT domain */
        poly_ntt(&s1.vec[l_idx]);
    }

    /* Matrix-vector multiplication */
    for (size_t k_idx = 0; k_idx < K; k_idx++) {
        poly t;
        {
            poly tmp_elem;
            // Sample the current element from A.
            expand_mat_elem(&tmp_elem, rho, k_idx, 0);
            poly_pointwise_montgomery(&t, &tmp_elem, &s1.vec[0]);
            for (size_t l_idx = 1; l_idx < L; l_idx++) {
                // Sample the element from A.
                expand_mat_elem(&tmp_elem, rho, k_idx, l_idx);
                poly_pointwise_montgomery(&t, &tmp_elem, &s1.vec[l_idx]);
                poly_add(&t, &t, &tmp_elem);
            }
        }
        poly_reduce(&t);
        poly_invntt_tomont(&t);

        /* Add error vector s2 */
        {
            poly s2;
             /* Sample short vector s2 */
            poly_uniform_eta(&s2, rhoprime, nonce++);
            pack_sk_s2(sk, &s2, k_idx);
            poly_add(&t, &t, &s2);
        }

        /* Compute t{0,1} */
        {
            poly t1, t0;
            poly_caddq(&t);
            poly_power2round(&t1, &t0, &t);
            pack_sk_t0(sk, &t0, k_idx);
            pack_pk_t1(pk, &t1, k_idx);
        }
    }

    /* Compute CRH(rho, t1) and write secret key */
    {
      /* Compute CRH(rho, t1) and write secret key */
      crh(tr, pk, CRYPTO_PUBLICKEYBYTES);
      pack_sk_tr(sk, tr);
    }
    return 0;
}
#endif

#ifndef X86
/* Does Pulpissimo have enough memory? */
int memcheck() {

    if(VAR_HARD_DATA_END >= ABS_DATA_END)
        return -1;

    return 0;
}
#else
int memcheck() {
  return 0;
}
#endif

/*************************************************
* Name:        crypto_sign_keypair
*
* Description: Generates public and private key.
*
* Arguments:   - uint8_t *pk: pointer to output public key (allocated
*                             array of CRYPTO_PUBLICKEYBYTES bytes)
*              - uint8_t *sk: pointer to output private key (allocated
*                             array of CRYPTO_SECRETKEYBYTES bytes)
*
* Returns 0 (success)
*************************************************/
int crypto_sign_keypair(uint8_t *pk, uint8_t *sk) {
  // TODO: move this to global mem
  uint8_t *seedbuf;
  uint8_t *tr;
  const uint8_t *rho, *rhoprime, *key;

  polyvecl *mat, *s1, *s1hat;
  polyveck *s2, *t1, *t0;

  int i, j;

  /* memmap */
#ifdef X86
  mat = &g_mat[0];
  s1 = &g_s1;
  s1hat = &g_s1hat;
  s2 = &g_s2;
  t1 = &g_t1;
  t0 = &g_t0;
#else
  seedbuf = (uint8_t*)KEYGEN_SEED_ADDR;
  tr = (uint8_t*)KEYGEN_TR_ADDR;
  mat = (polyvecl*)MATRIX_BASE_ADDR;
  s1 = (polyvecl*)S1_BASE_ADDR;
  s1hat = (polyvecl*)S1HAT_BASE_ADDR;
  s2 = (polyveck*)S2_BASE_ADDR;
  t1 = (polyveck*)T1_BASE_ADDR;
  t0 = (polyveck*)T0_BASE_ADDR;
#endif
  /* Get randomness for rho, rhoprime and key */
  randombytes(seedbuf, SEEDBYTES);
  
  
  shake256(seedbuf, 3*SEEDBYTES, seedbuf, SEEDBYTES);
  
  
  rho = seedbuf;
  rhoprime = seedbuf + SEEDBYTES;
  key = seedbuf + 2*SEEDBYTES;

  /* Expand matrix */
  
  polyvec_matrix_expand(mat, rho);

  /* Sample short vectors s1 and s2 */
  polyvecl_uniform_eta(s1, rhoprime, 0);
  
  
  polyveck_uniform_eta(s2, rhoprime, L);

  

  /* Matrix-vector multiplication */
  //s1hat = s1;
  // copy s1 to s1hat
  for(i = 0; i < L; i++)
  {
    for(j = 0; j<N; j++)
    {
      s1hat->vec[i].coeffs[j] = s1->vec[i].coeffs[j];
    }
  }

  
  polyvecl_ntt(s1hat);
  

  polyvec_matrix_pointwise_montgomery(t1, mat, s1hat);

  
  polyveck_reduce(t1);
  
  
  polyveck_invntt_tomont(t1);
  

  /* Add error vector s2 */
  polyveck_add(t1, t1, s2);

  /* Extract t1 and write public key */
  polyveck_caddq(t1);
  
  polyveck_power2round(t1, t0, t1);
  
  pack_pk(pk, rho, t1);
  
  
  /* Compute CRH(rho, t1) and write secret key */
  crh(tr, pk, CRYPTO_PUBLICKEYBYTES);

  
  
  pack_sk(sk, rho, tr, key, t0, s1, s2);

  return 0;
}


/*************************************************
* Name:        crypto_sign_signature
*
* Description: Computes signature.
*
* Arguments:   - uint8_t *sig:   pointer to output signature (of length CRYPTO_BYTES)
*              - size_t *siglen: pointer to output length of signature
*              - uint8_t *m:     pointer to message to be signed
*              - size_t mlen:    length of message
*              - uint8_t *sk:    pointer to bit-packed secret key
*
* Returns 0 (success)
**************************************************/
int crypto_sign_signature(uint8_t *sig,
                          unsigned int *siglen,
                          const uint8_t *m,
                          unsigned int mlen,
                          uint8_t *sk)
{
  unsigned int n;
  uint8_t *seedbuf;
  uint8_t *rho, *tr, *key, *mu, *rhoprime;
  uint16_t nonce = 0;
  polyvecl *mat, *s1, *y, *z;
  polyveck *t0, *s2, *w1, *w0, *h;
  poly *cp;
#ifndef HWX
  keccak_state state;
#endif
  int i, j;

// memmap
#ifdef X86
  mat = &g_mat[0];
  s1 = &g_s1;
  y = &g_s1hat;
  z = &g_z;
  s2 = &g_s2;
  t0 = &g_t0;
  w1 = &g_w1;
  w0 = &g_w0;
  h = &g_h;
  cp = &g_cp;
#else
  mat = (polyvecl*) MATRIX_BASE_ADDR;
  s1 = (polyvecl*) S1_BASE_ADDR;
  y = (polyvecl*) S1HAT_BASE_ADDR;
  z = (polyvecl*) Z_BASE_ADDR;
  t0 = (polyveck*) T0_BASE_ADDR;
  s2 = (polyveck*) S2_BASE_ADDR;
  w1 = (polyveck*) W1_BASE_ADDR;
  w0 = (polyveck*) W0_BASE_ADDR;
  h = (polyveck*) H_BASE_ADDR;
  cp = (poly*)CP_BASE_ADDR;
  seedbuf = (uint8_t*)SEEDBUF_ADDR;
#endif

  rho = seedbuf;
  tr = rho + SEEDBYTES;
  key = tr + CRHBYTES;
  mu = key + SEEDBYTES;
  rhoprime = mu + CRHBYTES;

  unpack_sk(rho, tr, key, t0, s1, s2, sk);

#ifndef HWX
  /* Compute CRH(tr, msg) */
  shake256_init(&state);
  shake256_absorb(&state, tr, CRHBYTES);
  shake256_absorb(&state, m, mlen);
  shake256_finalize(&state);
  shake256_squeeze(mu, CRHBYTES, &state);
#else
/* Compute CRH(tr, msg) */
  shake256_init();
  shake256_absorb(tr, CRHBYTES);
  shake256_absorb(m, mlen);     // word misalignment possible, current implementaion only supports if the mlen is aligned as a word, mlem = 4,8,12
  shake256_finalize();
  shake256_squeeze(mu, CRHBYTES);
#endif

#ifdef DILITHIUM_RANDOMIZED_SIGNING
  randombytes(rhoprime, CRHBYTES);
#else
  crh(rhoprime, key, SEEDBYTES + CRHBYTES);
#endif

  /* Expand matrix and transform vectors */
  polyvec_matrix_expand(mat, rho);


  polyvecl_ntt(s1);
  polyveck_ntt(s2);
  polyveck_ntt(t0);


rej:
  /* Sample intermediate vector y */
  polyvecl_uniform_gamma1(y, rhoprime, nonce++);


  // copy y to z
  for(i = 0; i < L; i++)
  {
    for(j = 0; j<N; j++)
    {
      z->vec[i].coeffs[j] = y->vec[i].coeffs[j];
    }
  }

  polyvecl_ntt(z);


  /* Matrix-vector multiplication */
  polyvec_matrix_pointwise_montgomery(w1, mat, z);
  polyveck_reduce(w1);

  polyveck_invntt_tomont(w1);


  /* Decompose w and call the random oracle */
  polyveck_caddq(w1);
  polyveck_decompose(w1, w0, w1);

  polyveck_pack_w1(sig, w1);



#ifndef HWX
  shake256_init(&state);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_absorb(&state, sig, K*POLYW1_PACKEDBYTES);
  shake256_finalize(&state);
  shake256_squeeze(sig, SEEDBYTES, &state);
#else
  shake256_init();
  shake256_absorb(mu, CRHBYTES);
  shake256_absorb(sig, K*POLYW1_PACKEDBYTES);
  shake256_finalize();
  shake256_squeeze(sig, SEEDBYTES);
#endif

  poly_challenge(cp, sig);

  poly_ntt(cp);


  /* Compute z, reject if it reveals secret */
  polyvecl_pointwise_poly_montgomery(z, cp, s1);


  polyvecl_invntt_tomont(z);

  polyvecl_add(z, z, y);
  /** TODO: Implement early abort */
  polyvecl_reduce(z);
  if(polyvecl_chknorm(z, GAMMA1 - BETA))
    goto rej;

  /* Check that subtracting cs2 does not change high bits of w and low bits
   * do not reveal secret information */
  polyveck_pointwise_poly_montgomery(h, cp, s2);

  polyveck_invntt_tomont(h);

  polyveck_sub(w0, w0, h);
  /** TODO: Implement early abort */
  polyveck_reduce(w0);
  if(polyveck_chknorm(w0, GAMMA2 - BETA))
    goto rej;

  /* Compute hints for w1 */
  polyveck_pointwise_poly_montgomery(h, cp, t0);

  polyveck_invntt_tomont(h);

  /** TODO: Implement early abort */
  polyveck_reduce(h);
  if(polyveck_chknorm(h, GAMMA2))
    goto rej;

  polyveck_add(w0, w0, h);
  polyveck_caddq(w0);
  n = polyveck_make_hint(h, w0, w1);
  if(n > OMEGA)
    goto rej;

  /* Write signature */
  pack_sig(sig, sig, z, h);
  *siglen = CRYPTO_BYTES;

  return 0;
}

/*************************************************
* Name:        crypto_sign
*
* Description: Compute signed message.
*
* Arguments:   - uint8_t *sm: pointer to output signed message (allocated
*                             array with CRYPTO_BYTES + mlen bytes),
*                             can be equal to m
*              - size_t *smlen: pointer to output length of signed
*                               message
*              - const uint8_t *m: pointer to message to be signed
*              - size_t mlen: length of message
*              - const uint8_t *sk: pointer to bit-packed secret key
*
* Returns 0 (success)
**************************************************/
int crypto_sign(uint8_t *sm,
                unsigned int *smlen,
                const uint8_t *m,
                unsigned int mlen,
                uint8_t *sk)
{
  unsigned int i;

  sk = (unsigned char*)SK_BASE_ADDR;
  m = (unsigned char*)M_BASE_ADDR;
  sm = (unsigned char*)SM_BASE_ADDR;

  for(i = 0; i < mlen; ++i)
    sm[CRYPTO_BYTES + mlen - 1 - i] = m[mlen - 1 - i];
  //crypto_sign_signature(sm, smlen, sm + CRYPTO_BYTES, mlen, sk);
  crypto_sign_signature(sm, smlen, m, mlen, sk);
  *smlen += mlen;
  return 0;
}

/*************************************************
* Name:        crypto_sign_verify
*
* Description: Verifies signature.
*
* Arguments:   - uint8_t *m: pointer to input signature
*              - size_t siglen: length of signature
*              - const uint8_t *m: pointer to message
*              - size_t mlen: length of message
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signature could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_verify(const uint8_t *sig,
                       unsigned int siglen,
                       const uint8_t *m,
                       unsigned int mlen,
                       const uint8_t *pk)
{
  unsigned int i;
  uint8_t *buf;
  uint8_t *rho;
  uint8_t *mu;
  uint8_t *c;
  uint8_t *c2;

  poly *cp;
  polyvecl *mat, *z;
  polyveck *t1, *w1, *h;
#ifndef HWX
  keccak_state state;
#endif
  // memmap
#ifdef X86
  mat = &g_mat[0];
  z = &g_z;
  t1 = &g_t1;
  w1 = &g_w1;
  h = &g_h;
  cp = &g_cp;
#else
  mat = (polyvecl*) MATRIX_BASE_ADDR;
  z = (polyvecl*) Z_BASE_ADDR;
  t1 = (polyveck*) T1_BASE_ADDR;
  w1 = (polyveck*) W1_BASE_ADDR;
  h = (polyveck*) H_BASE_ADDR;
  cp = (poly*) CP_BASE_ADDR;

  buf = (uint8_t*)SIGN_BUF_BASE;
  rho = buf + K*POLYW1_PACKEDBYTES;
  mu = rho + SEEDBYTES;
  c = mu + CRHBYTES;
  c2 = c + SEEDBYTES;
#endif

  if(siglen != CRYPTO_BYTES)
    return -1;

  unpack_pk(rho, t1, pk);

  if(unpack_sig(c, z, h, sig))
    return -2;
  if(polyvecl_chknorm(z, GAMMA1 - BETA))
    return -3;



  /* Compute CRH(CRH(rho, t1), msg) */
  crh(mu, pk, CRYPTO_PUBLICKEYBYTES);
#ifndef HWX
  shake256_init(&state);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_absorb(&state, m, mlen);
  shake256_finalize(&state);
  shake256_squeeze(mu, CRHBYTES, &state);
#else
  shake256_init();
  shake256_absorb(mu, CRHBYTES);
  shake256_absorb(m, mlen);   // FIXME: word misalignment possible,
                              // temporary solution.
                              //        first absorb aligned words then use the Peripheral bus to transmit the remaining
                              //        alt, use a simple logic at the end 
  shake256_finalize();
  shake256_squeeze(mu, CRHBYTES);
#endif



  /* Matrix-vector multiplication; compute Az - c2^dt1 */


  poly_challenge(cp, c);
  polyvec_matrix_expand(mat, rho);



  polyvecl_ntt(z);

  polyvec_matrix_pointwise_montgomery(w1, mat, z);


  poly_ntt(cp);

  polyveck_shiftl(t1);

  polyveck_ntt(t1);

  polyveck_pointwise_poly_montgomery(t1, cp, t1);

  polyveck_sub(w1, w1, t1);
  polyveck_reduce(w1);

  polyveck_invntt_tomont(w1);


  /* Reconstruct w1 */
  polyveck_caddq(w1);
  polyveck_use_hint(w1, w1, h);
  polyveck_pack_w1(buf, w1);


  /* Call random oracle and verify challenge */
#ifndef HWX
  shake256_init(&state);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_absorb(&state, buf, K*POLYW1_PACKEDBYTES);
  shake256_finalize(&state);
  shake256_squeeze(c2, SEEDBYTES, &state);
#else
  shake256_init();
  shake256_absorb(mu, CRHBYTES);
  shake256_absorb(buf, K*POLYW1_PACKEDBYTES);
  shake256_finalize();
  shake256_squeeze(c2, SEEDBYTES);
#endif


  for(i = 0; i < SEEDBYTES; ++i)
    if(c[i] != c2[i])
      return -4;
  return 0;
}

/*************************************************
* Name:        crypto_sign_open
*
* Description: Verify signed message.
*
* Arguments:   - uint8_t *m: pointer to output message (allocated
*                            array with smlen bytes), can be equal to sm
*              - size_t *mlen: pointer to output length of message
*              - const uint8_t *sm: pointer to signed message
*              - size_t smlen: length of signed message
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signed message could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_open(uint8_t *m,
                     unsigned int *mlen,
                     const uint8_t *sm,
                     unsigned int smlen,
                     const uint8_t *pk)
{
  unsigned int i;
  int ret;

  // declare all pointer here
  m = (unsigned char*)M1_BASE_ADDR;
  pk = (unsigned char*)PK_BASE_ADDR;
  sm = (unsigned char*)SM_BASE_ADDR;

  if(smlen < CRYPTO_BYTES)
    goto badsig;

  *mlen = smlen - CRYPTO_BYTES;
  // FIXME: WORD ALIGN
  // manually copy msg from sig to the m buf to avoid access conflict
  for (i = 0; i < *mlen; i++)
  {
    m[i] = sm[CRYPTO_BYTES + i];
  }
  //ret = crypto_sign_verify(sm, CRYPTO_BYTES, sm + CRYPTO_BYTES, *mlen, pk);
  ret = crypto_sign_verify(sm, CRYPTO_BYTES, m, *mlen, pk);
  if(ret)
    goto badsig;
  else {
    /* All good, copy msg, return 0 */
    // for(i = 0; i < *mlen; ++i)
    //   m[i] = sm[CRYPTO_BYTES + i];
    return 0;
  }

badsig:
  /* Signature verification failed */
  *mlen = -1;
  for(i = 0; i < smlen; ++i)
    m[i] = 0;

  return ret;
}