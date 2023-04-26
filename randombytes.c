#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "randombytes.h"

void randombytes(uint8_t *out, size_t outlen) {
	int i;
	for (i=0; i < outlen; i++)
		out[i] = (uint8_t)i;
}
