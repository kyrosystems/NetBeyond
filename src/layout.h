#ifndef NETBEYOND_LAYOUT_H
#define NETBEYOND_LAYOUT_H
#include <stddef.h>
#define NB_LAYOUT_MAX_LINES 512
#define NB_LAYOUT_LINE_MAX 256
typedef struct { char text[NB_LAYOUT_MAX_LINES][NB_LAYOUT_LINE_MAX]; size_t count; int truncated; } NB_Layout;
int nb_layout_text(const char *text, size_t columns, NB_Layout *layout);
#endif
