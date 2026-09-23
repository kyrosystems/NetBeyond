#include "../src/http.h"
#include <assert.h>
int main(void) {
    const char plain[] = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nhi";
    const char chunked[] = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n2\r\nhi\r\n0\r\n\r\n";
    NB_HttpResponse response;
    assert(nb_http_parse_response(plain, sizeof plain - 1, &response));
    assert(response.status == 200 && response.body_bytes == 2 && !response.chunked);
    assert(nb_http_parse_response(chunked, sizeof chunked - 1, &response));
    assert(response.chunked);
    assert(!nb_http_parse_response("not http", 8, &response));
    return 0;
}
