#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "elm_priv.h"

char *
_str_ncpy(char *dest, const char *src, size_t count)
{
   if ((!dest) || (!src)) return NULL;
   return strncpy(dest, src, count);
}

char *
_str_append(char *str, const char *txt, int *len, int *alloc)
{
   int txt_len = strlen(txt);

   if (txt_len <= 0) return str;
   if ((*len + txt_len) >= *alloc)
     {
        char *str2;
        int alloc2;

        alloc2 = *alloc + txt_len + 128;
        str2 = realloc(str, alloc2);
        if (!str2) return str;
        *alloc = alloc2;
        str = str2;
     }
   strcpy(str + *len, txt);
   *len += txt_len;
   return str;
}

char *
_elm_util_mkup_to_text(const char *mkup)
{
   return evas_textblock_text_markup_to_utf8(NULL, mkup);
}

char *
_elm_util_text_to_mkup(const char *text)
{
   return evas_textblock_text_utf8_to_markup(NULL, text);
}

double
_elm_atof(const char *s)
{
   char *cradix, *buf, *p;
   
   if ((!s) || (!s[0])) return 0.0;
   cradix = nl_langinfo(RADIXCHAR);
   if (!cradix) return atof(s);
   buf = alloca(strlen(s) + 1);
   strcpy(buf, s);
   for (p = buf; *p; p++)
     {
        if (*p == '.') *p = *cradix;
     }
   return atof(buf);
}
