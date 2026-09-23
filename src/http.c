#include "http.h"
#include <ctype.h>
#include <string.h>

static int starts_ci(const char *text, size_t length, const char *prefix) {
    size_t i;
    for (i = 0; prefix[i]; ++i) {
        if (i >= length || tolower((unsigned char)text[i]) != prefix[i]) return 0;
    }
    return 1;
}

int nb_http_parse_response(const char *data, size_t size, NB_HttpResponse *response) {
    size_t i = 0;
    size_t line_start = 0;
    size_t header_end = 0;
    int status = 0;
    if (!data || !response || size < 12 || memcmp(data, "HTTP/", 5)) return 0;
    memset(response, 0, sizeof *response);
    while (i < size && data[i] != ' ') ++i;
    while (i < size && data[i] == ' ') ++i;
    while (i < size && isdigit((unsigned char)data[i])) { status = status * 10 + data[i] - '0'; ++i; }
    if (status < 100 || status > 999) return 0;
    for (i = 0; i + 3 < size; ++i) if (data[i] == '\r' && data[i+1] == '\n' && data[i+2] == '\r' && data[i+3] == '\n') { header_end = i + 4; break; }
    if (!header_end) return 0;
    for (i = 0; i < header_end; ++i) {
        size_t end = i;
        while (end + 1 < header_end && !(data[end] == '\r' && data[end+1] == '\n')) ++end;
        if (end > line_start && starts_ci(data + line_start, end - line_start, "transfer-encoding:")) {
            size_t j;
            for (j = line_start; j + 7 <= end; ++j) if (starts_ci(data + j, end - j, "chunked")) response->chunked = 1;
        }
        line_start = end + 2;
        i = end + 1;
    }
    response->status = status;
    response->header_bytes = header_end;
    response->body_bytes = size - header_end;
    return 1;
}
