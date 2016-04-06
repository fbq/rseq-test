#!/bin/sh

CC=gcc
CFLAGS="-pthread -Wall -O2 -g"

RUNLIST="benchmark-cpuid-thread-local-abi
benchmark-cpuid-thread-local-abi-lazy
benchmark-cpuid-baseline
benchmark-cpuid-glibc-getcpu
benchmark-cpuid-inline-getcpu
benchmark-cpuid-syscall
benchmark-cpuid-gs"

${CC} ${CFLAGS} -DCONFIG_THREAD_LOCAL_ABI -o benchmark-cpuid-thread-local-abi benchmark-cpuid.c
${CC} ${CFLAGS} -DCONFIG_THREAD_LOCAL_ABI_LAZY -o benchmark-cpuid-thread-local-abi-lazy benchmark-cpuid.c
${CC} ${CFLAGS} -DCONFIG_BASELINE -o benchmark-cpuid-baseline benchmark-cpuid.c
${CC} ${CFLAGS} -DCONFIG_GLIBC_GETCPU -o benchmark-cpuid-glibc-getcpu benchmark-cpuid.c
${CC} ${CFLAGS} -DCONFIG_INLINE_GETCPU -o benchmark-cpuid-inline-getcpu benchmark-cpuid.c
${CC} ${CFLAGS} -DCONFIG_GETCPU_SYSCALL -o benchmark-cpuid-syscall benchmark-cpuid.c
${CC} ${CFLAGS} -DCONFIG_GETCPU_GS -o benchmark-cpuid-gs benchmark-cpuid.c

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
