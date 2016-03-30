#!/bin/sh

CC=gcc
CFLAGS="-pthread -Wall -O2 -g"

RUNLIST="getcpu-thread-local-abi
getcpu-thread-local-abi-lazy
getcpu-baseline
getcpu-glibc-getcpu
getcpu-inline-getcpu
getcpu-syscall
getcpu-gs"

${CC} ${CFLAGS} -DCONFIG_THREAD_LOCAL_ABI -o getcpu-thread-local-abi getcpu.c
${CC} ${CFLAGS} -DCONFIG_THREAD_LOCAL_ABI_LAZY -o getcpu-thread-local-abi-lazy getcpu.c
${CC} ${CFLAGS} -DCONFIG_BASELINE -o getcpu-baseline getcpu.c
${CC} ${CFLAGS} -DCONFIG_GLIBC_GETCPU -o getcpu-glibc-getcpu getcpu.c
${CC} ${CFLAGS} -DCONFIG_INLINE_GETCPU -o getcpu-inline-getcpu getcpu.c
${CC} ${CFLAGS} -DCONFIG_GETCPU_SYSCALL -o getcpu-syscall getcpu.c
${CC} ${CFLAGS} -DCONFIG_GETCPU_GS -o getcpu-gs getcpu.c

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


