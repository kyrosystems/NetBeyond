#ifndef NETBEYOND_HTML_H
#define NETBEYOND_HTML_H
#include <stddef.h>
#define NB_DOCUMENT_TITLE_MAX 256
#define NB_DOCUMENT_TEXT_MAX 16384
#define NB_DOCUMENT_LINKS_MAX 64
#define NB_DOCUMENT_URL_MAX 2048
typedef struct { char href[NB_DOCUMENT_URL_MAX]; char label[256]; } NB_Link;
typedef struct { char title[NB_DOCUMENT_TITLE_MAX]; char text[NB_DOCUMENT_TEXT_MAX]; NB_Link links[NB_DOCUMENT_LINKS_MAX]; size_t link_count; int truncated; } NB_Document;
int nb_html_parse(const char *html, size_t length, NB_Document *document);
#endif
