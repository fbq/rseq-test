/*
 * example-rseq-cpuid-lazy.c
 *
 * Copyright (c) 2015-2016 Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sched.h>
#include <sys/syscall.h>
#include <stddef.h>
#include "rseq.h"

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

__thread int rseq_init_fail;

static pthread_key_t rseq_key;

static void
destroy_rseq_key(void *key)
{
	if (rseq_unregister_current_thread())
		abort();
}

static int32_t
read_cpu_id(void)
{
	int32_t cpu;

	cpu = rseq_current_cpu_raw();
	if (unlikely(cpu < 0)) {
		if (rseq_init_fail)
			goto fallback;
		/*
		 * Note: would need to disable signals across register
		 * and pthread_setspecific to be signal-safe.
		 */
		if (!rseq_register_current_thread()) {
			/*
			 * Register destroy notifier. Pointer needs to
			 * be non-NULL.
			 */
			if (pthread_setspecific(rseq_key, (void *)0x1))
				abort();
		} else {
			rseq_init_fail = 1;
			fprintf(stderr, "Unable to initialize restartable sequences.\n");
			fprintf(stderr, "Using sched_getcpu() as fallback.\n");
			goto fallback;
		}
		cpu = rseq_current_cpu();
	}
	return cpu;

fallback:
	return sched_getcpu();
}

int
main(int argc, char **argv)
{
	printf("Current CPU number: %d\n", read_cpu_id());

	exit(EXIT_SUCCESS);
}

static void __attribute__((constructor))
rseq_cpuid_lazy_init(void)
{
	int ret;

	ret = pthread_key_create(&rseq_key, destroy_rseq_key);
	if (ret) {
		errno = -ret;
		perror("pthread_key_create");
		abort();
	}
}

static void __attribute__((destructor))
rseq_cpuid_lazy_destroy(void)
{
	int ret;

	ret = pthread_key_delete(rseq_key);
	if (ret) {
		errno = -ret;
		perror("pthread_key_delete");
		abort();
	}
}
