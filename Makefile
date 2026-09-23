CC ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Werror
.PHONY: test clean
test: build/test_core
	./build/test_core
build/test_core: src/core.c src/core.h tests/test_core.c
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ src/core.c tests/test_core.c
clean:
	rm -rf build
