#include <stdio.h>
#include "ntt.h"
#include "poly.h"
#include "fips202.h"

#include "pulp.h"

#define MEM_ADDR        0x1c040000
#define SIZEOFPOLY      256

//#define BENCH_PWM

//#define BENCH_NTT

//#define BENCH_SHAKE

#define BENCH_EXAPANDA


// SHAKE
#define ABSBYTES        256
#define DESBYTES        256


void bench_ntt()
{
    uint32_t *a;

    int i;

    a = (uint32_t*) MEM_ADDR;

    for (i=0; i<256;i++){
        a[i] = i;
    }

    reset_ntt_monitors();

    ntt(a);

    printf("#CC: %d\n", nttCycleCount());
}

void bench_shake()
{
    uint32_t *shake_in = (uint32_t*) MEM_ADDR;
	uint32_t *shake_out = shake_in + ABSBYTES;
    int i;

    for (i=0; i<ABSBYTES;i++){
        shake_in[i] = i;
    }

    reset_shake_monitors();

    printf("#CC: %d\n", shakeCycleCount());

    shake256((uint8_t*)shake_out, DESBYTES, (uint8_t*)shake_in, ABSBYTES + 32);

    printf("#CC: %d\n", shakeCycleCount());
}

void bench_pwm()
{

    poly *a, *b, *c;
    int i;

    a = (poly*)MEM_ADDR;
    b = a + 1;
    c = b + 1;

    for(i=0;i<256;i++) {
        a->coeffs[i] = i;
        b->coeffs[i] = i;
        c->coeffs[i] = 0;
    }

    printf("%x %x %x \n", a,b,c);

    reset_pwm_monitors();

    poly_pointwise_montgomery(c,a,b);

    printf("#CC: %d\n", pwmCycleCount());

}

void bench_expanda()
{
    int i;
    poly *a;
    uint8_t *seed;
    uint16_t nonce;

    seed = (uint8_t*)MEM_ADDR;
    a = (poly*)(seed + SEEDBYTES);
    nonce = 1;

    for(i=0;i<SEEDBYTES;i++) {
        seed[i] = i;
    }    

    reset_pwm_monitors();
    poly_uniform(a,seed,nonce);

    printf("#CC: %d\n", pwmCycleCount());
}

int main()
{
    // Initiate performance counters
    perf_start();

#ifdef BENCH_PWM
    bench_pwm();
#endif
#ifdef BENCH_NTT
    bench_ntt();
#endif

#ifdef BENCH_SHAKE
    bench_shake();
#endif

#ifdef BENCH_EXAPANDA
    bench_expanda();
#endif

    return 0;
}