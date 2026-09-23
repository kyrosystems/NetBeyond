#ifndef NETBEYOND_HTTP_H
#define NETBEYOND_HTTP_H
#include <stddef.h>
typedef struct { int status; size_t header_bytes; size_t body_bytes; int chunked; } NB_HttpResponse;
int nb_http_parse_response(const char *data, size_t size, NB_HttpResponse *response);
#endif
