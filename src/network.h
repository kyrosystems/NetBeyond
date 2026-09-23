#ifndef NETBEYOND_NETWORK_H
#define NETBEYOND_NETWORK_H
#include <stddef.h>
#define NB_NETWORK_MAX_BODY (1024 * 1024)
typedef struct { int status; size_t size; char body[NB_NETWORK_MAX_BODY + 1]; } NB_NetworkResponse;
int nb_network_get(const char *url, NB_NetworkResponse *response);
#endif
