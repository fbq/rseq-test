

.PHONY: clean

clean:
	rm -f benchmark-cpuid-thread-local-abi \
		benchmark-cpuid-thread-local-abi-lazy \
		benchmark-cpuid-baseline \
		benchmark-cpuid-glibc-benchmark-cpuid \
		benchmark-cpuid-inline-benchmark-cpuid \
		benchmark-cpuid-syscall \
		benchmark-cpuid-gs
