#!/bin/bash

TESTS="s i M I C p"

NR_THREADS=1

for a in ${TESTS}; do
	echo ./benchmark-rseq -T $a -t ${NR_THREADS} -r 500000000
	time ./benchmark-rseq -T $a -t ${NR_THREADS} -r 500000000
done

NR_THREADS=8

for a in ${TESTS}; do
	echo ./benchmark-rseq -T $a -t ${NR_THREADS} -r 500000000
	time ./benchmark-rseq -T $a -t ${NR_THREADS} -r 500000000
done
