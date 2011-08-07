#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "elm_priv.h"

static char *
_str_ncpy(char *dest, const char *src, size_t count)
{
   if ((!dest) || (!src)) return NULL;
   return strncpy(dest, src, count);
}

static char *
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
   char *str = NULL;
   int str_len = 0, str_alloc = 0;
   char *s, *p;
   char *tag_start, *tag_end, *esc_start, *esc_end, *ts;

   if (!mkup) return NULL;
   tag_start = tag_end = esc_start = esc_end = NULL;
   p = (char *)mkup;
   s = p;
   for (;;)
     {
        if ((!*p) ||
            (tag_end) || (esc_end) ||
            (tag_start) || (esc_start))
          {
             if (tag_end)
               {
                  char *ttag;

                  ttag = malloc(tag_end - tag_start);
                  if (ttag)
                    {
                       _str_ncpy(ttag, tag_start + 1, tag_end - tag_start - 1);
                       ttag[tag_end - tag_start - 1] = 0;
                       if (!strcmp(ttag, "br"))
                         str = _str_append(str, "\n", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "\n"))
                         str = _str_append(str, "\n", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "\\n"))
                         str = _str_append(str, "\n", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "\t"))
                         str = _str_append(str, "\t", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "\\t"))
                         str = _str_append(str, "\t", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "ps")) /* Unicode paragraph separator */
                         str = _str_append(str, "\xE2\x80\xA9", &str_len, &str_alloc);
                       free(ttag);
                    }
                  tag_start = tag_end = NULL;
               }
             else if (esc_end)
               {
                  ts = malloc(esc_end - esc_start + 1);
                  if (ts)
                    {
                       const char *esc;
                       _str_ncpy(ts, esc_start, esc_end - esc_start);
                       ts[esc_end - esc_start] = 0;
                       esc = evas_textblock_escape_string_get(ts);
                       if (esc)
                         str = _str_append(str, esc, &str_len, &str_alloc);
                       free(ts);
                    }
                  esc_start = esc_end = NULL;
               }
             else if ((!*p) && (s))
               {
                  ts = malloc(p - s + 1);
                  if (ts)
                    {
                       _str_ncpy(ts, s, p - s);
                       ts[p - s] = 0;
                       str = _str_append(str, ts, &str_len, &str_alloc);
                       free(ts);
                    }
               }

             if (!*p) break;
          }
        if (*p == '<')
          {
             if ((s) && (!esc_start))
               {
                  tag_start = p;
                  tag_end = NULL;
                  ts = malloc(p - s + 1);
                  if (ts)
                    {
                       _str_ncpy(ts, s, p - s);
                       ts[p - s] = 0;
                       str = _str_append(str, ts, &str_len, &str_alloc);
                       free(ts);
                    }
                  s = NULL;
               }
          }
        else if (*p == '>')
          {
             if (tag_start)
               {
                  tag_end = p;
                  s = p + 1;
               }
          }
        else if (*p == '&')
          {
             if ((s) && (!tag_start))
               {
                  esc_start = p;
                  esc_end = NULL;
                  ts = malloc(p - s + 1);
                  if (ts)
                    {
                       _str_ncpy(ts, s, p - s);
                       ts[p - s] = 0;
                       str = _str_append(str, ts, &str_len, &str_alloc);
                       free(ts);
                    }
                  s = NULL;
               }
          }
        else if (*p == ';')
          {
             if (esc_start)
               {
                  esc_end = p + 1;
                  s = p + 1;
               }
          }
        p++;
     }
   return str;
}

char *
_elm_util_text_to_mkup(const char *text)
{
   char *str = NULL;
   int str_len = 0, str_alloc = 0;
   int ch, pos = 0, pos2 = 0;

   if (!text) return NULL;
   for (;;)
     {
        pos = pos2;
        pos2 = evas_string_char_next_get((char *)(text), pos2, &ch);
        if ((ch <= 0) || (pos2 <= 0)) break;
        if (ch == '\n')
          str = _str_append(str, "<br>", &str_len, &str_alloc);
        else if (ch == '\t')
          str = _str_append(str, "<\t>", &str_len, &str_alloc);
        else if (ch == '<')
          str = _str_append(str, "&lt;", &str_len, &str_alloc);
        else if (ch == '>')
          str = _str_append(str, "&gt;", &str_len, &str_alloc);
        else if (ch == '&')
          str = _str_append(str, "&amp;", &str_len, &str_alloc);
        else if (ch == 0x2029) /* PS */
          str = _str_append(str, "<ps>", &str_len, &str_alloc);
        else
          {
             char tstr[16];

             _str_ncpy(tstr, text + pos, pos2 - pos);
             tstr[pos2 - pos] = 0;
             str = _str_append(str, tstr, &str_len, &str_alloc);
          }
     }
   return str;
}
