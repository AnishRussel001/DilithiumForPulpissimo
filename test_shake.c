#include <stdio.h>
#include "fips202.h"

#include "pulp.h"


#define ABS_BYTES       32
#define DESORB_BYTES    32

#define MEM_ADDR        0x1c040000

int main()
{

    uint32_t *abs, *des;
    int i;

    abs = (uint32_t*)MEM_ADDR;
    des = abs + ABS_BYTES;

    for (i=0; i<ABS_BYTES;i++){
        abs[i] = i;
    }

    // Initiate performance counters
    perf_start();
    reset_shake_monitors();

    shake128((uint8_t*)des, DESORB_BYTES, (uint8_t*)abs, ABS_BYTES);

    printf("#CC: %d\n", shakeCycleCount());

    return 0;
}