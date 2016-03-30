#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sched.h>
#include <sys/syscall.h>
#include <stddef.h>
#include "thread_local_abi.h"

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#define __NR_thread_local_abi       326

static inline int
thread_local_abi(uint32_t tlabi_nr,
		volatile struct thread_local_abi *tlabi,
		uint32_t feature_mask, int flags)
{
	return syscall(__NR_thread_local_abi, tlabi_nr, tlabi,
			feature_mask, flags);
}

/*
 * __thread_local_abi is recommended as symbol name for the thread-local ABI.
 * Weak attribute is recommended when declaring this variable in libraries.
 */
__thread __attribute__((weak))
volatile struct thread_local_abi __thread_local_abi;

static
int tlabi_cpu_id_register(void)
{
	if (thread_local_abi(0, &__thread_local_abi, TLABI_FEATURE_CPU_ID, 0))
		return -1;
	return 0;
}

static
int32_t read_cpu_id(void)
{
	if (unlikely(!(__thread_local_abi.features & TLABI_FEATURE_CPU_ID)))
		return sched_getcpu();
	return __thread_local_abi.cpu_id;
}

int
main(int argc, char **argv)
{
	if (tlabi_cpu_id_register()) {
		fprintf(stderr, "Unable to initialize thread-local ABI cpu_id feature.\n");
		fprintf(stderr, "Using sched_getcpu() as fallback.\n");
	}
	printf("Current CPU number: %d\n", read_cpu_id());
	printf("TLABI features: 0x%x\n", __thread_local_abi.features);
	
	exit(EXIT_SUCCESS);
}
