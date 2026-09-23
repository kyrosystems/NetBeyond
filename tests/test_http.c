#include "../src/http.h"
#include <assert.h>
int main(void){const char response[]="HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nhi";NB_HttpResponse parsed;assert(nb_http_parse_response(response,sizeof response-1,&parsed));assert(parsed.status==200);assert(parsed.body_bytes==2);return 0;}
