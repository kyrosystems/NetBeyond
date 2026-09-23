CC ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Werror
.PHONY: test clean
test: build/test_core build/test_history
	./build/test_core
	./build/test_history
build/test_core: src/core.c src/core.h tests/test_core.c
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ src/core.c tests/test_core.c
build/test_history: src/history.c src/history.h tests/test_history.c
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ src/history.c tests/test_history.c
clean:
	rm -rf build
