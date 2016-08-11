/*
 * benchmark-cpuid.c
 *
 * Get current CPU ID benchmark
 *
 * Copyright (c) 2015 Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define _GNU_SOURCE

#include <error.h>
#include <signal.h>
#include <stdio.h>
#include <ucontext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <inttypes.h>
#include <urcu/system.h>
#include <urcu/compiler.h>
#include <urcu/uatomic.h>
#include <sys/prctl.h>
#ifdef CONFIG_GETCPU_GS
#include <asm/prctl.h>
#endif
#include "rseq.h"

#ifndef min
#define min(a, b)	((a) < (b) ? (a) : (b))
#endif

#ifdef __linux__
#include <syscall.h>
#endif

#if defined(_syscall0)
_syscall0(pid_t, gettid)
#elif defined(__NR_gettid)
#include <unistd.h>
static inline pid_t gettid(void)
{
	return syscall(__NR_gettid);
}
#else
#include <sys/types.h>
#include <unistd.h>

/* Fall-back on getpid for tid if not available. */
static inline pid_t gettid(void)
{
	return getpid();
}
#endif

#ifdef CONFIG_RSEQ
static const char *config_name = "rseq.cpu_id";
#elif CONFIG_RSEQ_LAZY
static const char *config_name = "rseq.cpu_id (lazy)";
#elif CONFIG_BASELINE
static const char *config_name = "baseline";
#elif CONFIG_GLIBC_GETCPU
static const char *config_name = "glibc getcpu";
#elif CONFIG_INLINE_GETCPU
static const char *config_name = "inline getcpu lsl";
#elif CONFIG_GETCPU_SYSCALL
static inline int sys_getcpu(void)
{
	return syscall(__NR_getcpu, NULL, NULL);
}
static const char *config_name = "getcpu syscall";
#elif CONFIG_GETCPU_GS
static const char *config_name = "getcpu gs";
#else
#error "Uknown config."
#endif

//#define NR_THREADS	64
//#define NR_THREADS	16
//#define NR_THREADS	8
//#define NR_THREADS	4
#define NR_THREADS	1
/* For testing with multiple threads per cpu */
//#define NR_CPUS		64
//#define NR_CPUS		32
//#define NR_CPUS		8
#define NR_CPUS		8
//#define NR_CPUS		4

__thread int rseq_init_fail;

static volatile int test_go, test_stop;

static unsigned int delay_loop;

#ifdef CONFIG_GETCPU_GS
static inline int sys_arch_prctl_set_gs(unsigned long *addr)
{
	return syscall(__NR_arch_prctl, ARCH_SET_GS, addr);
}
#else
static inline int sys_arch_prctl_set_gs(unsigned long *addr)
{
	return 0;
}
#endif

__thread volatile int output;

static uint64_t thread_loops[NR_THREADS];

static void set_affinity(int cpu)
{
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	if (sched_setaffinity(0, sizeof(mask), &mask)) {
		perror("sched_setaffinity");
		abort();
	}
}

static void signal_off_save(sigset_t *oldset)
{
	sigset_t set;
	int ret;

	sigfillset(&set);
	ret = pthread_sigmask(SIG_BLOCK, &set, oldset);
	if (ret)
		abort();
}

static void signal_restore(sigset_t oldset)
{
	int ret;

	ret = pthread_sigmask(SIG_SETMASK, &oldset, NULL);
	if (ret)
		abort();
}

#define sigsafe_fprintf(...)			\
	({					\
		sigset_t __set;			\
		int __ret;			\
						\
		signal_off_save(&__set);	\
		__ret = fprintf(__VA_ARGS__);	\
		signal_restore(__set);		\
		__ret;				\
	})

#define sigsafe_fprintf_dbg(...)

static
void do_delay_loop(int nr)
{
	int i;

	for (i = 0; i < nr; i++)
		caa_cpu_relax();
}

#define GDT_ENTRY_PER_CPU		15
#define __PER_CPU_SEG			(GDT_ENTRY_PER_CPU*8 + 3)
static inline unsigned int __getcpu(void)
{
	unsigned int p;
	asm volatile ("lsl %1,%0" : "=r" (p) : "r" (__PER_CPU_SEG));
	return p;
}

static unsigned long dummygs;

//Dummy test for gs read.
static inline int getcpu_gs(void)
{
	unsigned int p;
	asm volatile ("mov %%gs:0,%0" : "=r" (p));
	return p;
}

__thread uint32_t example_feature_fail_mask;

static pthread_key_t rseq_key;

static void
destroy_rseq_key(void *key)
{
	if (rseq_unregister_current_thread())
		abort();
}

static inline int read_cpu_id_lazy(void)
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

static void *thread_fct(void *arg)
{
	int ret;
	int thread_nr = (long) arg;
	int cpu = thread_nr % NR_CPUS;
	uint64_t loop_count = 0;
	int cputest = 0;

	set_affinity(cpu);

	sigsafe_fprintf(stderr, "[tid: %d, cpu: %d] Thread starts\n",
		gettid(), cpu);
	ret = rseq_register_current_thread();
	if (ret) {
		abort();
	}

	while (!test_go) {
	}
	cmm_smp_mb();

	if (sys_arch_prctl_set_gs(&dummygs)) {
		perror("arch_prctl");
		exit(EXIT_FAILURE);
	}

	for (;;) {
#ifdef CONFIG_RSEQ
		cputest = rseq_current_cpu_raw();
#elif CONFIG_RSEQ_LAZY
		cputest = read_cpu_id_lazy();
#elif CONFIG_BASELINE
		/* no-op */
#elif CONFIG_GLIBC_GETCPU
		cputest = sched_getcpu();
#elif CONFIG_INLINE_GETCPU
		cputest = __getcpu();
#elif CONFIG_GETCPU_SYSCALL
		cputest = sys_getcpu();
#elif CONFIG_GETCPU_GS
		cputest = getcpu_gs();
#else
#error "Uknown config."
#endif
		asm volatile ("" : : "r" (cputest));
		do_delay_loop(delay_loop);
		loop_count++;
		if (caa_unlikely(test_stop)) {
			break;
		}
	}
	thread_loops[thread_nr] = loop_count;
	if (rseq_unregister_current_thread())
		abort();
	return NULL;
}

int main(int argc, char **argv)
{
	int i;
	int err;
	pthread_t tid[NR_THREADS];
	void *tret;
	uint64_t tot_loops = 0;
	unsigned int remain, duration;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s [seconds] [delay_loop]\n",
			argv[0]);
		goto error;
	}
	duration = atoi(argv[1]);
	if (duration < 0) {
		fprintf(stderr, "Use positive integer for seconds\n");
		goto error;
	}
	delay_loop = atoi(argv[2]);
	if (delay_loop < 0) {
		fprintf(stderr, "Use positive integer for delay_loop\n");
		goto error;
	}

	for (i = 0; i < NR_THREADS; i++) {
		err = pthread_create(&tid[i], NULL, thread_fct,
			(void *) (long) i);
		if (err != 0)
			goto error;
	}

	cmm_smp_mb();

	test_go = 1;

	remain = duration;
	do {
		remain = sleep(remain);
	} while (remain > 0);

	test_stop = 1;

	for (i = 0; i < NR_THREADS; i++) {
		err = pthread_join(tid[i], &tret);
		if (err != 0)
			goto error;
		tot_loops += thread_loops[i];
	}

	fprintf(stderr, "SUMMARY: [%s] %u threads, %u cores, %u delay loops, %" PRIu64 " loops / %d s  (%4g ns/[loops/core])\n",
		config_name, NR_THREADS, NR_CPUS, delay_loop,
		tot_loops, duration,
		(double) duration * min(NR_CPUS, NR_THREADS) * 1000000000ULL/ (double) tot_loops);

	exit(EXIT_SUCCESS);

error:
	exit(EXIT_FAILURE);
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
