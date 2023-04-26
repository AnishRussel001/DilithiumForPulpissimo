/**
 * benchmarking Dilithium components
 */

#include <stdio.h>
#include "pulp.h"
#include "api.h"
#include "randombytes.h"

#define USE_PRECOMPUTED_KEYS

#ifdef USE_PRECOMPUTED_KEYS
#include "test_vector.h"
#endif

#define MLEN		50
#define KAT_CRYPTO_FAILURE -1

void printCharArray(unsigned char *array, int size) {
	int i;
	
	if (size==0)
		printf("empty");
	
	for (i=0;i<size;i++)
		printf("%x ", array[i]);
	printf("\n");
}

int main (void) {
	int ret_val;
#ifndef USE_PRECOMPUTED_KEYS
    unsigned char pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    unsigned char sm[MLEN + CRYPTO_BYTES];
#endif
	unsigned char m[MLEN], m1[MLEN + CRYPTO_BYTES];
	unsigned long long smlen, mlen, mlen1;

    if (memcheck() < 0) {
        printf("Poly data Memory overflow!\n");
        return -1;
    }
    else {
        printf("Memory check: Success!\n");
    }

    printf("Random message generation..\n");
    randombytes(m, MLEN);

#ifndef USE_PRECOMPUTED_KEYS
    printf("Generating Keypair\n");
    
    perf_start();
    if ( (ret_val = crypto_sign_keypair(pk, sk)) != 0) {
        printf("crypto_sign_keypair returned <%d>\n", ret_val);
        return -1;
    }
    perf_stop();
    printf("Keygen #CC: %d\n", cpu_perf_get(CSR_PCER_CYCLES));
    printf("Keygen #INST: %d\n", cpu_perf_get(CSR_PCER_INSTR));
    perf_reset();

    printf("pk: ");
    printCharArray(pk, CRYPTO_PUBLICKEYBYTES);
    printf("sk:");
    printCharArray(sk, CRYPTO_SECRETKEYBYTES);

    printf("Signing...\n");
    perf_start();
    if ( (ret_val = crypto_sign(sm, &smlen, m, MLEN, sk)) != 0) {
        printf("crypto_sign returned <%d>\n", ret_val);
        return KAT_CRYPTO_FAILURE;
    }
    perf_stop();
    printf("Signature Generation #CC: %d\n", cpu_perf_get(CSR_PCER_CYCLES));
    printf("Signature Generation #INST: %d\n", cpu_perf_get(CSR_PCER_INSTR));
    perf_reset();
    printf("signed message: \n");
    printCharArray(sm, smlen);    
#endif

#ifdef USE_PRECOMPUTED_KEYS
    smlen = sizeof(sm);
#endif
    int i;
    printf("pk: ");
    for(i=0; i<32; i++)
        printf("%x ", pk[i]);
    printf("\n");

    perf_start();
    printf("Verifying signed message of length %d...\n", smlen);
    if ( (ret_val = crypto_sign_open(m1, &mlen1, sm, smlen, pk)) != 0) {
        printf("crypto_sign_open returned <%d>\n", ret_val);
        return -1;
    }
    perf_stop();
    printf("Signature Verification #CC: %d\n", cpu_perf_get(CSR_PCER_CYCLES));
    printf("Signature Verification #INST: %d\n", cpu_perf_get(CSR_PCER_INSTR));
    perf_reset();
    
    if ( MLEN != mlen1 ) {
        printf("crypto_sign_open returned bad 'mlen': Got <%d>, expected <%d>\n", mlen1, MLEN);
        return -2;
    }
    
    if ( memcmp(m, m1, (unsigned int)MLEN) ) {
        printf("crypto_sign_open returned bad 'm' value\n");
        return -3;
    }

	printf("Simulation SUCCESS!\n");
	return 0;
}
