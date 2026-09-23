#ifndef NETBEYOND_HISTORY_H
#define NETBEYOND_HISTORY_H
#include <stddef.h>
#define NB_HISTORY_CAPACITY 128
#define NB_HISTORY_URL_MAX 2048
typedef struct { char entries[NB_HISTORY_CAPACITY][NB_HISTORY_URL_MAX]; size_t count; } NB_History;
void nb_history_init(NB_History *history);
int nb_history_add(NB_History *history, const char *url);
const char *nb_history_get(const NB_History *history, size_t index);
int nb_history_load(NB_History *history, const char *path);
int nb_history_save(const NB_History *history, const char *path);
#endif
