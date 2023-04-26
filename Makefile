SOURCES = sign.c packing.c  poly.c polyvec.c ntt.c reduce.c rounding.c
KECCAK_SOURCES = $(SOURCES) fips202.c symmetric-shake.c

PULP_ARCH_CFLAGS    =  -march=rv32imc -DRV_ISA_RV32
PULP_ARCH_LDFLAGS   =  -march=rv32imc
PULP_ARCH_OBJDFLAGS =  -Mmarch=rv32imc

PULP_APP = dilithium

# testing SHAKE
PULP_APP_SRCS=test.c randombytes.c $(KECCAK_SOURCES)
# Dilithium
#PULP_APP_SRCS = keygen.c randombytes.c $(KECCAK_SOURCES)
#Nothing
#PULP_APP_SRCS = benchmark.c randombytes.c $(KECCAK_SOURCES)
#PULP_CFLAGS = -O3 -g

include $(PULPRT_HOME)/rules/pulp.mk
