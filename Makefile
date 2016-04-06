# Copyright (C) 2016  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
#
# THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
# OR IMPLIED. ANY USE IS AT YOUR OWN RISK.
#
# Permission is hereby granted to use or copy this program for any
# purpose, provided the above notices are retained on all copies.
# Permission to modify the code and to distribute modified code is
# granted, provided the above notices are retained, and a notice that
# the code was modified is included with the above copyright notice.

CPPFLAGS = -O2 -g

all: example-tlabi-cpuid example-tlabi-cpuid-lazy test-tlabi-cpuid

example-tlabi-cpuid: example-tlabi-cpuid.c
	$(CC) $(CPPFLAGS) -o $@ $<

example-tlabi-cpuid-lazy: example-tlabi-cpuid-lazy.c
	$(CC) $(CPPFLAGS) -o $@ $<

test-tlabi-cpuid: test-tlabi-cpuid.c
	$(CC) $(CPPFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f benchmark-cpuid-thread-local-abi \
		benchmark-cpuid-thread-local-abi-lazy \
		benchmark-cpuid-baseline \
		benchmark-cpuid-glibc-getcpu \
		benchmark-cpuid-inline-getcpu \
		benchmark-cpuid-syscall \
		benchmark-cpuid-gs \
		example-tlabi-cpuid \
		example-tlabi-cpuid-lazy \
		test-tlabi-cpuid