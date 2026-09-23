#include "../src/history.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    NB_History h, loaded;
    nb_history_init(&h);
    assert(nb_history_add(&h, "https://one"));
    assert(nb_history_add(&h, "https://two"));
    assert(nb_history_add(&h, "https://one"));
    assert(h.count == 2);
    assert(!strcmp(nb_history_get(&h, 0), "https://one"));
    assert(nb_history_save(&h, "test-history.tmp"));
    assert(nb_history_load(&loaded, "test-history.tmp"));
    assert(loaded.count == 2);
    assert(!strcmp(nb_history_get(&loaded, 0), "https://one"));
    remove("test-history.tmp");
    return 0;
}
