#include <stdint.h>
#include "params.h"
#include "ntt.h"
#include "reduce.h"

//#define HWX

#include "pulp.h"
#include "polymemlayout.h"

#ifdef HWX

#define NTT_BASE_ADDR       0x0C
#define NTT_TWD_BASE_ADDR   0x10
#define NTT_MODE            0x14

#define NTT_TRIG            0x18
#define NTT_STAT            0x1C

#define NTT_TWD_TRIG        0x20
#define NTT_TWD_STAT        0x24

#define ARCHI_HWPE_ADDR_BASE        0x1A10C000

/* useful macros */
#define HWPE_WRITE(value, offset)   pulp_write32(ARCHI_HWPE_ADDR_BASE+offset , value)
#define HWPE_READ(offset)           pulp_read32(ARCHI_HWPE_ADDR_BASE + offset)
#endif

static const int32_t zetas[N] = {
         0,    25847, -2608894,  -518909,   237124,  -777960,  -876248,   466468,
   1826347,  2353451,  -359251, -2091905,  3119733, -2884855,  3111497,  2680103,
   2725464,  1024112, -1079900,  3585928,  -549488, -1119584,  2619752, -2108549,
  -2118186, -3859737, -1399561, -3277672,  1757237,   -19422,  4010497,   280005,
   2706023,    95776,  3077325,  3530437, -1661693, -3592148, -2537516,  3915439,
  -3861115, -3043716,  3574422, -2867647,  3539968,  -300467,  2348700,  -539299,
  -1699267, -1643818,  3505694, -3821735,  3507263, -2140649, -1600420,  3699596,
    811944,   531354,   954230,  3881043,  3900724, -2556880,  2071892, -2797779,
  -3930395, -1528703, -3677745, -3041255, -1452451,  3475950,  2176455, -1585221,
  -1257611,  1939314, -4083598, -1000202, -3190144, -3157330, -3632928,   126922,
   3412210,  -983419,  2147896,  2715295, -2967645, -3693493,  -411027, -2477047,
   -671102, -1228525,   -22981, -1308169,  -381987,  1349076,  1852771, -1430430,
  -3343383,   264944,   508951,  3097992,    44288, -1100098,   904516,  3958618,
  -3724342,    -8578,  1653064, -3249728,  2389356,  -210977,   759969, -1316856,
    189548, -3553272,  3159746, -1851402, -2409325,  -177440,  1315589,  1341330,
   1285669, -1584928,  -812732, -1439742, -3019102, -3881060, -3628969,  3839961,
   2091667,  3407706,  2316500,  3817976, -3342478,  2244091, -2446433, -3562462,
    266997,  2434439, -1235728,  3513181, -3520352, -3759364, -1197226, -3193378,
    900702,  1859098,   909542,   819034,   495491, -1613174,   -43260,  -522500,
   -655327, -3122442,  2031748,  3207046, -3556995,  -525098,  -768622, -3595838,
    342297,   286988, -2437823,  4108315,  3437287, -3342277,  1735879,   203044,
   2842341,  2691481, -2590150,  1265009,  4055324,  1247620,  2486353,  1595974,
  -3767016,  1250494,  2635921, -3548272, -2994039,  1869119,  1903435, -1050970,
  -1333058,  1237275, -3318210, -1430225,  -451100,  1312455,  3306115, -1962642,
  -1279661,  1917081, -2546312, -1374803,  1500165,   777191,  2235880,  3406031,
   -542412, -2831860, -1671176, -1846953, -2584293, -3724270,   594136, -3776993,
  -2013608,  2432395,  2454455,  -164721,  1957272,  3369112,   185531, -1207385,
  -3183426,   162844,  1616392,  3014001,   810149,  1652634, -3694233, -1799107,
  -3038916,  3523897,  3866901,   269760,  2213111,  -975884,  1717735,   472078,
   -426683,  1723600, -1803090,  1910376, -1667432, -1104333,  -260646, -3833893,
  -2939036, -2235985,  -420899, -2286327,   183443,  -976891,  1612842, -3545687,
   -554416,  3919660,   -48306, -1362209,  3937738,  1400424,  -846154,  1976782
};

#define ENABLE_BENCH
#ifdef ENABLE_BENCH
unsigned int ntt_cycle_monitor = 0;
unsigned int begin, end;

void reset_ntt_monitors()
{
  ntt_cycle_monitor = 0;
}

#define START_BENCH     (begin = cpu_perf_get(0))

#define END_BENCH       (ntt_cycle_monitor = ntt_cycle_monitor + (cpu_perf_get(0)  - begin))

unsigned int nttCycleCount()
{
  return ntt_cycle_monitor;
}
#else
void reset_ntt_monitors()
{
}

#define START_BENCH

#define END_BENCH

unsigned int nttCycleCount()
{
  return 0;
}
#endif

#ifdef HWX
/* update twddle factor */
void updatetwiddle()
{

  uint32_t *twdBase = (uint32_t*) TWD_BASE;
  int i, status;

  // copy zeta to the RAM

  for (i = 0; i < N ; i++) {
    twdBase[i] = zetas[i];
  }

  // configure and trigger
	HWPE_WRITE((unsigned int)twdBase, NTT_TWD_BASE_ADDR);
	HWPE_WRITE(0x01, NTT_TWD_TRIG);
	// wait for the operation end
	while(1)
	{
		status = HWPE_READ(NTT_TWD_STAT);
		if (status == 0)
			break;
	}

}

#else
void updatetwiddle()
{

}
#endif


#ifndef HWX
/*************************************************
* Name:        ntt
*
* Description: Forward NTT, in-place. No modular reduction is performed after
*              additions or subtractions. Output vector is in bitreversed order.
*
* Arguments:   - uint32_t p[N]: input/output coefficient array
**************************************************/
void ntt(int32_t a[N]) {
  START_BENCH;
  unsigned int len, start, j, k;
  int32_t zeta, t;

  k = 0;
  for(len = 128; len > 0; len >>= 1) {
    for(start = 0; start < N; start = j + len) {
      zeta = zetas[++k];
      for(j = start; j < start + len; ++j) {
        t = montgomery_reduce((int64_t)zeta * a[j + len]);
        a[j + len] = a[j] - t;
        a[j] = a[j] + t;
      }
    }
  }

  END_BENCH;
}

#else
/** NTT Hardware acceleration */
void ntt(int32_t *a) {

  START_BENCH;

  int status;
  // configuration and trigger
	HWPE_WRITE((unsigned int)a, NTT_BASE_ADDR);
  HWPE_WRITE(0, NTT_MODE); //NTT
	HWPE_WRITE(0x01, NTT_TRIG);
	
	while(1)
	{
		status = HWPE_READ(NTT_STAT);
		if (status == 0)
			break;
	}
  END_BENCH;
}
#endif

/*************************************************
* Name:        invntt_tomont
*
* Description: Inverse NTT and multiplication by Montgomery factor 2^32.
*              In-place. No modular reductions after additions or
*              subtractions; input coefficients need to be smaller than
*              Q in absolute value. Output coefficient are smaller than Q in
*              absolute value.
*
* Arguments:   - uint32_t p[N]: input/output coefficient array
**************************************************/

#ifndef HWX
void invntt_tomont(int32_t a[N]) {
  START_BENCH;
  unsigned int start, len, j, k;
  int32_t t, zeta;
  const int32_t f = 41978; // mont^2/256

  k = 256;
  for(len = 1; len < N; len <<= 1) {
    for(start = 0; start < N; start = j + len) {
      zeta = -zetas[--k];
      for(j = start; j < start + len; ++j) {
        t = a[j];
        a[j] = t + a[j + len];
        a[j + len] = t - a[j + len];
        a[j + len] = montgomery_reduce((int64_t)zeta * a[j + len]);
      }
    }
  }
  END_BENCH;
  for(j = 0; j < N; ++j) {
    a[j] = montgomery_reduce((int64_t)f * a[j]);
  }
}
#else
/** NTT Hardware acceleration */
void invntt_tomont(int32_t *a) {
  START_BENCH;
  int status, j;
  const int32_t f = 41978; // mont^2/256
  // configuration and trigger
	HWPE_WRITE((unsigned int)a, NTT_BASE_ADDR);
  HWPE_WRITE(1, NTT_MODE); //NTT
	HWPE_WRITE(0x01, NTT_TRIG);
	
	while(1)
	{
		status = HWPE_READ(NTT_STAT);
		if (status == 0)
			break;
	}
  END_BENCH;

  // software montgomery
  for(j = 0; j < N; ++j) {
    a[j] = montgomery_reduce((int64_t)f * a[j]);
  }
}
#endif
