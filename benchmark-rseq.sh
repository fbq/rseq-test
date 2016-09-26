#!/bin/bash

TESTS_PERCPU="s i c p P"
TESTS_GLOBAL="M I C"

NR_THREADS=1

#baseline
for a in "b"; do
	echo ./benchmark-rseq -T $a -t ${NR_THREADS} -r 1000000000
	time ./benchmark-rseq -T $a -t ${NR_THREADS} -r 1000000000
done

for a in ${TESTS_PERCPU}; do
	echo ./benchmark-rseq -T $a -t ${NR_THREADS} -r 1000000000
	time ./benchmark-rseq -T $a -t ${NR_THREADS} -r 1000000000
done

for a in ${TESTS_GLOBAL}; do
	echo ./benchmark-rseq -T $a -t ${NR_THREADS} -r 1000000000
	time ./benchmark-rseq -T $a -t ${NR_THREADS} -r 1000000000
done


NR_THREADS=8

for a in ${TESTS_PERCPU}; do
	echo ./benchmark-rseq -T $a -t ${NR_THREADS} -r 1000000000
	time ./benchmark-rseq -T $a -t ${NR_THREADS} -r 1000000000
done

for a in ${TESTS_GLOBAL}; do
	echo ./benchmark-rseq -T $a -t ${NR_THREADS} -r 100000000
	time ./benchmark-rseq -T $a -t ${NR_THREADS} -r 100000000
done
