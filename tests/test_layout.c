#include "../src/layout.h"
#include <assert.h>
#include <string.h>
int main(void){NB_Layout l;assert(nb_layout_text("one two three four",7,&l));assert(l.count==3);assert(!strcmp(l.text[0],"one two"));assert(!strcmp(l.text[1],"three"));assert(!strcmp(l.text[2],"four"));return 0;}
