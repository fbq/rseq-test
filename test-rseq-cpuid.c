/*
 * test-rseq-cpuid.c
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
#include <error.h>
#include <stddef.h>
#include "rseq.h"

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

static uint64_t nr_loops = 100000;
static int32_t affine_cpu = 0;
static int nr_cpus;

static int update_affinity(void)
{
	int ret;
	cpu_set_t mask;

	affine_cpu = (affine_cpu + 1) % nr_cpus;
	CPU_ZERO(&mask);
	CPU_SET(affine_cpu, &mask);
	ret = sched_setaffinity(0, sizeof(mask), &mask);
	if (ret) {
		perror("sched_setaffinity");
		return -1;
	}
	return 0;
}

static int test_cpu_nr(void)
{
	int32_t cpu;

	cpu = rseq_current_cpu_raw();
	if (cpu != affine_cpu) {
		fprintf(stderr, "[error] Current CPU number %d differs from CPU affinity to CPU %d.\n",
			cpu, affine_cpu);
		return -1;
	}
	return 0;
}

static int do_test_loop(void)
{
	if (update_affinity())
		return -1;
	if (test_cpu_nr())
		return -1;
	return 0;
}

int main(int argc, char **argv)
{
	uint64_t i;

	nr_cpus = sysconf(_SC_NPROCESSORS_CONF);
	if (nr_cpus <= 0) {
		fprintf(stderr, "[error] Unable to get number of configured processors.\n");
		exit(EXIT_FAILURE);
	}
	if (nr_cpus == 1)
		fprintf(stderr, "[warning] This won't test much on a uniprocessor kernel.\n");

	/* Set initial affinity to CPU 1 on multiprocessor systems. */
	if (update_affinity())
		exit(EXIT_FAILURE);

	printf("# Registering restartable sequences.\n");
	if (rseq_register_current_thread()) {
		fprintf(stderr, "[error] Unable to initialize restartable sequences.\n");
		perror("rseq_register_current_thread");
		exit(EXIT_FAILURE);
	}

	if (test_cpu_nr())
		exit(EXIT_FAILURE);

	printf("# Hopping across CPUs by setting affinity, check that cpu_id is consistent.\n");
	for (i = 0; i < nr_loops; i++) {
		if (do_test_loop())
			exit(EXIT_FAILURE);
	}

	printf("All OK!\n");

	if (rseq_unregister_current_thread()) {
		perror("rseq_unregister_current_thread");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
