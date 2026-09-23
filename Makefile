CC ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Werror
.PHONY: test clean
test: build/test_core build/test_history build/test_html build/test_layout
	./build/test_core
	./build/test_history
	./build/test_html
	./build/test_layout
build/test_core: src/core.c tests/test_core.c
	mkdir -p build; $(CC) $(CFLAGS) -o $@ src/core.c tests/test_core.c
build/test_history: src/history.c tests/test_history.c
	mkdir -p build; $(CC) $(CFLAGS) -o $@ src/history.c tests/test_history.c
build/test_html: src/html.c tests/test_html.c
	mkdir -p build; $(CC) $(CFLAGS) -o $@ src/html.c tests/test_html.c
build/test_layout: src/layout.c tests/test_layout.c
	mkdir -p build; $(CC) $(CFLAGS) -o $@ src/layout.c tests/test_layout.c
clean:
	rm -rf build
