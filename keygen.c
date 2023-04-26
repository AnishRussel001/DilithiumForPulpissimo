#include <stdio.h>
#include "api.h"
#include "randombytes.h"

//#define USE_PRECOMPUTED_KEYS
#include "polymemlayout.h"  

#include "ntt.h"
#include "fips202.h"
#include "poly.h"

#ifndef X86
#include "pulp.h"
#endif

#ifdef USE_PRECOMPUTED_KEYS
#include "test_vector.h"
#endif

#define ENABLE_KEY_PRINT

void printCharArray(unsigned char *array, int size) {
	int i;
	
	if (size==0)
		printf("empty");
	
	for (i=0;i<size;i++)
		printf("%x ", array[i]);
	printf("\n");
}

int main (void) {
	int ret_val, i;

#ifndef X86
    uint32_t cc_begin, cc_end, ins_begin, ins_end;
    uint32_t cc_keypair, cc_sign, cc_verify, ins_keypair, ins_sign, ins_verify;
#endif
#ifndef X86
    unsigned char *sk = (unsigned char*)SK_BASE_ADDR;
    unsigned char *pk = (unsigned char*)PK_BASE_ADDR; 
    unsigned char *sm = (unsigned char*)SM_BASE_ADDR;
    unsigned char *m1 = (unsigned char*)M1_BASE_ADDR;
    unsigned char *m = (unsigned char*)M_BASE_ADDR;
#else
    unsigned char pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    unsigned char sm[MLEN + CRYPTO_BYTES], m1[MLEN + CRYPTO_BYTES];
    unsigned char m[MLEN];
#endif
	unsigned int mlen, smlen, mlen1;

    if (memcheck() < 0) {
        printf("Poly data Memory overflow!\n");
        return -1;
    }
    else {
        printf("Memory check: Success!\n");
    }

    printf("Initiating Twiddle Update..\n");
    updatetwiddle();

    printf("Random message generation..\n");
    randombytes(m, MLEN);
    
#ifndef USE_PRECOMPUTED_KEYS
#ifndef X86
    printf("Generating Keypair\n");
    // Initiate Performance counter
    reset_ntt_monitors();
    reset_shake_monitors();
    reset_pwm_monitors();
    perf_start();
    cc_begin = cpu_perf_get(0);
    ins_begin = cpu_perf_get(2);
#endif

    if ( (ret_val = crypto_sign_keypair(pk, sk)) != 0) {
        printf("crypto_sign_keypair returned <%d>\n", ret_val);
        return -1;
    }
#ifndef X86
    // Collect keypair perfomance results
    cc_end = cpu_perf_get(0);
    ins_end = cpu_perf_get(2);
    cc_keypair = cc_end - cc_begin;
    ins_keypair = ins_end - ins_begin;
    perf_reset();
#endif

    printf("Keygen #CC: %d\n", cc_keypair);
	printf("Keygen #INST: %d\n", ins_keypair);

    printf("ntt count #CC: %d \n", nttCycleCount());
    printf("shake count #CC: %d \n", shakeCycleCount());
    printf("PWM count #CC: %d \n", pwmCycleCount());
#ifdef ENABLE_KEY_PRINT
    printf("pk: ");
    printCharArray(pk, CRYPTO_PUBLICKEYBYTES);
    printf("sk:");
    printCharArray(sk, CRYPTO_SECRETKEYBYTES);
#endif

    printf("Signing...\n");
#ifndef X86
    // Performance counters before op
    reset_ntt_monitors();
    reset_shake_monitors();
    reset_pwm_monitors();
    cc_begin = cpu_perf_get(0);
    ins_begin = cpu_perf_get(2);

    // test
    start_timer();
    int test_start = get_time();
#endif

    if ( (ret_val = crypto_sign(sm, &smlen, m, MLEN, sk)) != 0) {
        printf("crypto_sign returned <%d>\n", ret_val);
        return KAT_CRYPTO_FAILURE;
    }

#ifndef X86
    // Collect sign perfomance results
    cc_end = cpu_perf_get(0);
    ins_end = cpu_perf_get(2);
    cc_sign = cc_end - cc_begin;
    ins_sign = ins_end - ins_begin;
    // test
    int test_end = get_time();
    int test_diff = test_end - test_start;
#endif

    printf("Sign #CC: %d\n", cc_sign);
	printf("Sign #INST: %d\n", ins_sign);

    printf("ntt count #CC: %d \n", nttCycleCount());
    printf("shake count #CC: %d \n", shakeCycleCount());
    printf("PWM count #CC: %d \n", pwmCycleCount());

    // test
    printf("test sign #CC: %d \n", test_diff);

    perf_reset();

#ifdef ENABLE_KEY_PRINT
    printf("signed message: \n");
    printCharArray(sm, smlen);    
#endif

#endif

#ifdef USE_PRECOMPUTED_KEYS
    smlen = sizeof(sm);
    // copy sm and pk to the RAM
#endif
    printf("Verifying signed message of length %d...\n", smlen);
#ifndef X86
    // Performance counters before op
    reset_ntt_monitors();
    reset_shake_monitors();
    reset_pwm_monitors();
    cc_begin = cpu_perf_get(0);
    ins_begin = cpu_perf_get(2);
#endif
    
    if ( (ret_val = crypto_sign_open(m1, &mlen1, sm, smlen, pk)) != 0) {
        printf("crypto_sign_open returned <%d>\n", ret_val);
        return KAT_CRYPTO_FAILURE;
    }

#ifndef X86
    // Collect verify perfomance results
    cc_end = cpu_perf_get(0);
    ins_end = cpu_perf_get(2);
    cc_verify = cc_end - cc_begin;
    ins_verify = ins_end - ins_begin;

    //Print benchmark results
    printf("Verify #CC: %d\n", cc_verify);
	printf("Verify #INST: %d\n", ins_verify);
    printf("ntt count #CC: %d \n", nttCycleCount());
    printf("shake count #CC: %d \n", shakeCycleCount());
    printf("PWM count #CC: %d \n", pwmCycleCount());
    perf_reset();
#endif
    if ( MLEN != mlen1 ) {
        printf("crypto_sign_open returned bad 'mlen': Got <%d>, expected <%d>\n", mlen1, MLEN);
        return KAT_CRYPTO_FAILURE;
    }
    
    int j;
    for (j = 0; j < MLEN; j++)
    {
        if (m[j] != m1[j])
            break;
    }

    if ( j != MLEN ) {
        printf("crypto_sign_open returned bad 'm' value\n");
        return KAT_CRYPTO_FAILURE;
    }
	printf("Simulation SUCCESS!\n");
	return 0;
}
