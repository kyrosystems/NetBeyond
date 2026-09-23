#include "core.h"
#include <ctype.h>
#include <string.h>

int nb_address(const char *input, char *output, size_t capacity) {
    const char *start, *end, *p;
    size_t n, used;
    int search = 0;
    if (!input || !output || !capacity) return -1;
    start = input;
    while (*start && isspace((unsigned char)*start)) ++start;
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) --end;
    n = (size_t)(end - start);
    if (!n) { output[0] = 0; return 0; }
    if ((n >= 7 && !strncmp(start, "http://", 7)) ||
        (n >= 8 && !strncmp(start, "https://", 8)) ||
        (n == 11 && !strncmp(start, "about:blank", 11))) {
        if (n >= capacity) return -1;
        memcpy(output, start, n); output[n] = 0; return 1;
    }
    if (memchr(start, ' ', n) || !memchr(start, '.', n)) search = 1;
    if (!search) {
        if (n + 9 > capacity) return -1;
        memcpy(output, "https://", 8); memcpy(output + 8, start, n);
        output[n + 8] = 0; return 1;
    }
    if (capacity < sizeof("https://www.bing.com/search?q=")) return -1;
    strcpy(output, "https://www.bing.com/search?q=");
    used = strlen(output);
    for (p = start; p < end; ++p) {
        unsigned char c = (unsigned char)*p;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            if (used + 2 > capacity) return -1;
            output[used++] = (char)c;
        } else {
            if (used + 4 > capacity) return -1;
            static const char hex[] = "0123456789ABCDEF";
            output[used++] = '%';
            output[used++] = hex[c >> 4];
            output[used++] = hex[c & 15];
        }
    }
    output[used] = 0;
    return 1;
}
