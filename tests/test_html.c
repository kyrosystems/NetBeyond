#include "../src/html.h"
#include <assert.h>
#include <string.h>
int main(void){const char p[]="<title>Hello &amp; Bye</title><p>One <b>two</b>.</p><a href='https://example.org'>Example</a><script>bad()</script>";NB_Document d;assert(nb_html_parse(p,sizeof p-1,&d));assert(!strcmp(d.title,"Hello&Bye"));assert(!strcmp(d.text,"One two. Example"));assert(d.link_count==1);assert(!strcmp(d.links[0].href,"https://example.org"));assert(!strcmp(d.links[0].label,"Example"));return 0;}
