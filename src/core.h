#ifndef NETBEYOND_CORE_H
#define NETBEYOND_CORE_H
#include <stddef.h>
/* Returns 1 for a navigable URL, 0 for empty input, -1 for insufficient space. */
int nb_address(const char *input, char *output, size_t capacity);
#endif
