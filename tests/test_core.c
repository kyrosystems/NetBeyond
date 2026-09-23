#include "../src/core.h"
#include <assert.h>
#include <string.h>
int main(void) {
    char out[256];
    assert(nb_address(" example.org ", out, sizeof out) == 1);
    assert(strcmp(out, "https://example.org") == 0);
    assert(nb_address("https://example.org/a", out, sizeof out) == 1);
    assert(strcmp(out, "https://example.org/a") == 0);
    assert(nb_address("a b", out, sizeof out) == 1);
    assert(strcmp(out, "https://www.bing.com/search?q=a%20b") == 0);
    assert(nb_address("", out, sizeof out) == 0);
    assert(nb_address("example.org", out, 2) == -1);
    return 0;
}
