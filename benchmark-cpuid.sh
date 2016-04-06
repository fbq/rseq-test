#!/bin/sh

CC=gcc
CFLAGS="-pthread -Wall -O2 -g"

RUNLIST="test-cpuid-thread-local-abi
test-cpuid-thread-local-abi-lazy
test-cpuid-baseline
test-cpuid-glibc-test-cpuid
test-cpuid-inline-test-cpuid
test-cpuid-syscall
test-cpuid-gs"

${CC} ${CFLAGS} -DCONFIG_THREAD_LOCAL_ABI -o test-cpuid-thread-local-abi test-cpuid.c
${CC} ${CFLAGS} -DCONFIG_THREAD_LOCAL_ABI_LAZY -o test-cpuid-thread-local-abi-lazy test-cpuid.c
${CC} ${CFLAGS} -DCONFIG_BASELINE -o test-cpuid-baseline test-cpuid.c
${CC} ${CFLAGS} -DCONFIG_GLIBC_GETCPU -o test-cpuid-glibc-test-cpuid test-cpuid.c
${CC} ${CFLAGS} -DCONFIG_INLINE_GETCPU -o test-cpuid-inline-test-cpuid test-cpuid.c
${CC} ${CFLAGS} -DCONFIG_GETCPU_SYSCALL -o test-cpuid-syscall test-cpuid.c
${CC} ${CFLAGS} -DCONFIG_GETCPU_GS -o test-cpuid-gs test-cpuid.c

for a in ${RUNLIST}; do
	echo "Running ./${a} 10 0"
	./${a} 10 0
done

#for a in ${RUNLIST}; do
#	echo "Running ./${a} 10 10"
#	./${a} 10 10
#done
#
#for a in ${RUNLIST}; do
#	echo "Running ./${a} 10 100"
#	./${a} 10 100
#done
