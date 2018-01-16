#ifndef _GFX_COLOR_UTIL_H
#define _GFX_COLOR_UTIL_H

#include <Eina.h>

EAPI int _gfx_color_format_clean_param(Eina_Tmpstr *s);
EAPI Eina_Bool _gfx_color_format_color_parse(const char *str, int slen, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a);

#endif