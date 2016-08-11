/*
 * example-rseq-cpuid.c
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
#include <stdbool.h>
#include "rseq.h"

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

int
main(int argc, char **argv)
{
	bool rseq_registered = false;

	if (!rseq_register_current_thread()) {
		rseq_registered = true;
	} else {
		fprintf(stderr, "Unable to initialize restartable sequences.\n");
		fprintf(stderr, "Using sched_getcpu() as fallback.\n");
	}
	printf("Current CPU number: %d\n", rseq_current_cpu());

	if (rseq_registered && rseq_unregister_current_thread()) {
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
