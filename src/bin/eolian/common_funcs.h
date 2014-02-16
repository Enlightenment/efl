#ifndef __EOLIAN_COMMON_FUNCS_H
#define __EOLIAN_COMMON_FUNCS_H

#include <Eina.h>

void _template_fill(Eina_Strbuf *buf, const char* templ, const char* classname, const char *funcname, Eina_Bool reset);

char *_nextline(char *str, unsigned int lines);

char *_startline(char *str, char *pos);

char *_first_line_get(const char *str);

#endif
