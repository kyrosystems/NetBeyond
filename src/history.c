#include "history.h"
#include <stdio.h>
#include <string.h>
void nb_history_init(NB_History *history) { if (history) history->count = 0; }
int nb_history_add(NB_History *history, const char *url) {
    size_t i, n;
    if (!history || !url || !url[0] || strlen(url) >= NB_HISTORY_URL_MAX) return 0;
    for (i = 0; i < history->count; ++i) if (!strcmp(history->entries[i], url)) {
        for (; i; --i) memcpy(history->entries[i], history->entries[i-1], NB_HISTORY_URL_MAX);
        strcpy(history->entries[0], url); return 1;
    }
    n = history->count < NB_HISTORY_CAPACITY ? history->count++ : NB_HISTORY_CAPACITY - 1;
    for (; n; --n) memcpy(history->entries[n], history->entries[n-1], NB_HISTORY_URL_MAX);
    strcpy(history->entries[0], url); return 1;
}
const char *nb_history_get(const NB_History *history, size_t index) { return (!history || index >= history->count) ? NULL : history->entries[index]; }
int nb_history_load(NB_History *history, const char *path) {
    FILE *f; char line[NB_HISTORY_URL_MAX];
    if (!history || !path || !(f = fopen(path, "r"))) return 0;
    nb_history_init(history);
    while (fgets(line, sizeof line, f)) { line[strcspn(line, "\r\n")] = 0; if (!nb_history_add(history, line)) { fclose(f); return 0; } }
    fclose(f); return 1;
}
int nb_history_save(const NB_History *history, const char *path) {
    FILE *f; size_t i;
    if (!history || !path || !(f = fopen(path, "w"))) return 0;
    for (i = history->count; i; --i) if (fprintf(f, "%s\n", history->entries[i-1]) < 0) { fclose(f); return 0; }
    return fclose(f) == 0;
}
