#include "config.h"
#include "Efl.h"

#define MY_CLASS EFL_TEXT_MARKUP_UTIL_CLASS

#define ERR(...) EINA_LOG_DOM_ERR(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#define _REPLACEMENT_CHAR 0xFFFC
#define _PARAGRAPH_SEPARATOR 0x2029
#define _NEWLINE '\n'
#define _TAB '\t'

#define _REPLACEMENT_CHAR_UTF8 "\xEF\xBF\xBC"
#define _PARAGRAPH_SEPARATOR_UTF8 "\xE2\x80\xA9"
#define _NEWLINE_UTF8 "\n"
#define _TAB_UTF8 "\t"
#define EVAS_TEXTBLOCK_IS_VISIBLE_FORMAT_CHAR(ch) \
   (((ch) == _REPLACEMENT_CHAR) || \
    ((ch) ==  _NEWLINE) || \
    ((ch) == _TAB) || \
    ((ch) == _PARAGRAPH_SEPARATOR))

#define _IS_TAB(item)                                             \
   (!strcmp(item, "tab") || !strcmp(item, "\t") || !strcmp(item, "\\t"))

#define _IS_LINE_SEPARATOR(item)                                             \
   (!strcmp(item, "br") || !strcmp(item, "\n") || !strcmp(item, "\\n"))

#define _IS_PARAGRAPH_SEPARATOR_SIMPLE(item)                                 \
   (!strcmp(item, "ps"))

#define _IS_PARAGRAPH_SEPARATOR(o, item)                                     \
   (_IS_PARAGRAPH_SEPARATOR_SIMPLE(item) ||                                  \
    (o->legacy_newline && _IS_LINE_SEPARATOR(item))) /* Paragraph separator */

static void
_markup_get_text_utf8_append(Eina_Strbuf *sbuf, const char *text)
{
   int ch, pos = 0, pos2 = 0;

   for (;;)
     {
        pos = pos2;
        ch = eina_unicode_utf8_next_get(text, &pos2);
        if ((ch <= 0) || (pos2 <= 0)) break;

        if (ch == _NEWLINE)
           eina_strbuf_append(sbuf, "<br/>");
        else if (ch == _TAB)
           eina_strbuf_append(sbuf, "<tab/>");
        else if (ch == '<')
           eina_strbuf_append(sbuf, "&lt;");
        else if (ch == '>')
           eina_strbuf_append(sbuf, "&gt;");
        else if (ch == '&')
           eina_strbuf_append(sbuf, "&amp;");
        else if (ch == '"')
           eina_strbuf_append(sbuf, "&quot;");
        else if (ch == _PARAGRAPH_SEPARATOR)
           eina_strbuf_append(sbuf, "<ps/>");
        else if (ch == _REPLACEMENT_CHAR)
           eina_strbuf_append(sbuf, "&#xfffc;");
        else if (ch != '\r')
          {
             eina_strbuf_append_length(sbuf, text + pos, pos2 - pos);
          }
     }
}

EOLIAN static char*
_efl_text_markup_util_text_to_markup(const char *text)
{
   Eina_Strbuf *sbuf;
   char *str = NULL;

   if (!text) return NULL;

   sbuf = eina_strbuf_new();

   _markup_get_text_utf8_append(sbuf, text);

   str = eina_strbuf_string_steal(sbuf);
   eina_strbuf_free(sbuf);
   return str;
}

/* table of html escapes (that i can find) this should be ordered with the
 * most common first as it's a linear search to match - no hash for this.
 *
 * these are stored as one large string and one additional array that
 * contains the offsets to the tokens for space efficiency.
 */
/**
 * @internal
 * @var escape_strings[]
 * This string consists of NULL terminated pairs of strings, the first of
 * every pair is an escape and the second is the value of the escape.
 */
static const char escape_strings[] =
/* most common escaped stuff */
"&quot;\0"     "\x22\0"
"&amp;\0"      "\x26\0"
"&apos;\0"     "\x27\0"
"&lt;\0"       "\x3c\0"
"&gt;\0"       "\x3e\0"
/* all the rest */
"&nbsp;\0"     "\xc2\xa0\0"
"&iexcl;\0"    "\xc2\xa1\0"
"&cent;\0"     "\xc2\xa2\0"
"&pound;\0"    "\xc2\xa3\0"
"&curren;\0"   "\xc2\xa4\0"
"&yen;\0"      "\xc2\xa5\0"
"&brvbar;\0"   "\xc2\xa6\0"
"&sect;\0"     "\xc2\xa7\0"
"&uml;\0"      "\xc2\xa8\0"
"&copy;\0"     "\xc2\xa9\0"
"&ordf;\0"     "\xc2\xaa\0"
"&laquo;\0"    "\xc2\xab\0"
"&not;\0"      "\xc2\xac\0"
"&shy;\0"      "\xc2\xad\0"
"&reg;\0"      "\xc2\xae\0"
"&macr;\0"     "\xc2\xaf\0"
"&deg;\0"      "\xc2\xb0\0"
"&plusmn;\0"   "\xc2\xb1\0"
"&sup2;\0"     "\xc2\xb2\0"
"&sup3;\0"     "\xc2\xb3\0"
"&acute;\0"    "\xc2\xb4\0"
"&micro;\0"    "\xc2\xb5\0"
"&para;\0"     "\xc2\xb6\0"
"&middot;\0"   "\xc2\xb7\0"
"&cedil;\0"    "\xc2\xb8\0"
"&sup1;\0"     "\xc2\xb9\0"
"&ordm;\0"     "\xc2\xba\0"
"&raquo;\0"    "\xc2\xbb\0"
"&frac14;\0"   "\xc2\xbc\0"
"&frac12;\0"   "\xc2\xbd\0"
"&frac34;\0"   "\xc2\xbe\0"
"&iquest;\0"   "\xc2\xbf\0"
"&Agrave;\0"   "\xc3\x80\0"
"&Aacute;\0"   "\xc3\x81\0"
"&Acirc;\0"    "\xc3\x82\0"
"&Atilde;\0"   "\xc3\x83\0"
"&Auml;\0"     "\xc3\x84\0"
"&Aring;\0"    "\xc3\x85\0"
"&Aelig;\0"    "\xc3\x86\0"
"&Ccedil;\0"   "\xc3\x87\0"
"&Egrave;\0"   "\xc3\x88\0"
"&Eacute;\0"   "\xc3\x89\0"
"&Ecirc;\0"    "\xc3\x8a\0"
"&Euml;\0"     "\xc3\x8b\0"
"&Igrave;\0"   "\xc3\x8c\0"
"&Iacute;\0"   "\xc3\x8d\0"
"&Icirc;\0"    "\xc3\x8e\0"
"&Iuml;\0"     "\xc3\x8f\0"
"&Eth;\0"      "\xc3\x90\0"
"&Ntilde;\0"   "\xc3\x91\0"
"&Ograve;\0"   "\xc3\x92\0"
"&Oacute;\0"   "\xc3\x93\0"
"&Ocirc;\0"    "\xc3\x94\0"
"&Otilde;\0"   "\xc3\x95\0"
"&Ouml;\0"     "\xc3\x96\0"
"&times;\0"    "\xc3\x97\0"
"&Oslash;\0"   "\xc3\x98\0"
"&Ugrave;\0"   "\xc3\x99\0"
"&Uacute;\0"   "\xc3\x9a\0"
"&Ucirc;\0"    "\xc3\x9b\0"
"&Yacute;\0"   "\xc3\x9d\0"
"&Thorn;\0"    "\xc3\x9e\0"
"&szlig;\0"    "\xc3\x9f\0"
"&agrave;\0"   "\xc3\xa0\0"
"&aacute;\0"   "\xc3\xa1\0"
"&acirc;\0"    "\xc3\xa2\0"
"&atilde;\0"   "\xc3\xa3\0"
"&auml;\0"     "\xc3\xa4\0"
"&aring;\0"    "\xc3\xa5\0"
"&aelig;\0"    "\xc3\xa6\0"
"&ccedil;\0"   "\xc3\xa7\0"
"&egrave;\0"   "\xc3\xa8\0"
"&eacute;\0"   "\xc3\xa9\0"
"&ecirc;\0"    "\xc3\xaa\0"
"&euml;\0"     "\xc3\xab\0"
"&igrave;\0"   "\xc3\xac\0"
"&iacute;\0"   "\xc3\xad\0"
"&icirc;\0"    "\xc3\xae\0"
"&iuml;\0"     "\xc3\xaf\0"
"&eth;\0"      "\xc3\xb0\0"
"&ntilde;\0"   "\xc3\xb1\0"
"&ograve;\0"   "\xc3\xb2\0"
"&oacute;\0"   "\xc3\xb3\0"
"&ocirc;\0"    "\xc3\xb4\0"
"&otilde;\0"   "\xc3\xb5\0"
"&ouml;\0"     "\xc3\xb6\0"
"&divide;\0"   "\xc3\xb7\0"
"&oslash;\0"   "\xc3\xb8\0"
"&ugrave;\0"   "\xc3\xb9\0"
"&uacute;\0"   "\xc3\xba\0"
"&ucirc;\0"    "\xc3\xbb\0"
"&uuml;\0"     "\xc3\xbc\0"
"&yacute;\0"   "\xc3\xbd\0"
"&thorn;\0"    "\xc3\xbe\0"
"&yuml;\0"     "\xc3\xbf\0"
"&alpha;\0"    "\xce\x91\0"
"&beta;\0"     "\xce\x92\0"
"&gamma;\0"    "\xce\x93\0"
"&delta;\0"    "\xce\x94\0"
"&epsilon;\0"  "\xce\x95\0"
"&zeta;\0"     "\xce\x96\0"
"&eta;\0"      "\xce\x97\0"
"&theta;\0"    "\xce\x98\0"
"&iota;\0"     "\xce\x99\0"
"&kappa;\0"    "\xce\x9a\0"
"&lambda;\0"   "\xce\x9b\0"
"&mu;\0"       "\xce\x9c\0"
"&nu;\0"       "\xce\x9d\0"
"&xi;\0"       "\xce\x9e\0"
"&omicron;\0"  "\xce\x9f\0"
"&pi;\0"       "\xce\xa0\0"
"&rho;\0"      "\xce\xa1\0"
"&sigma;\0"    "\xce\xa3\0"
"&tau;\0"      "\xce\xa4\0"
"&upsilon;\0"  "\xce\xa5\0"
"&phi;\0"      "\xce\xa6\0"
"&chi;\0"      "\xce\xa7\0"
"&psi;\0"      "\xce\xa8\0"
"&omega;\0"    "\xce\xa9\0"
"&hellip;\0"   "\xe2\x80\xa6\0"
"&euro;\0"     "\xe2\x82\xac\0"
"&larr;\0"     "\xe2\x86\x90\0"
"&uarr;\0"     "\xe2\x86\x91\0"
"&rarr;\0"     "\xe2\x86\x92\0"
"&darr;\0"     "\xe2\x86\x93\0"
"&harr;\0"     "\xe2\x86\x94\0"
"&larr;\0"     "\xe2\x87\x90\0"
"&rarr;\0"     "\xe2\x87\x92\0"
"&forall;\0"   "\xe2\x88\x80\0"
"&exist;\0"    "\xe2\x88\x83\0"
"&nabla;\0"    "\xe2\x88\x87\0"
"&prod;\0"     "\xe2\x88\x8f\0"
"&sum;\0"      "\xe2\x88\x91\0"
"&and;\0"      "\xe2\x88\xa7\0"
"&or;\0"       "\xe2\x88\xa8\0"
"&int;\0"      "\xe2\x88\xab\0"
"&ne;\0"       "\xe2\x89\xa0\0"
"&equiv;\0"    "\xe2\x89\xa1\0"
"&oplus;\0"    "\xe2\x8a\x95\0"
"&perp;\0"     "\xe2\x8a\xa5\0"
"&dagger;\0"   "\xe2\x80\xa0\0"
"&Dagger;\0"   "\xe2\x80\xa1\0"
"&bull;\0"     "\xe2\x80\xa2\0"
"&zwnj;\0"     "\xe2\x80\x8c\0"
"&zwj;\0"      "\xe2\x80\x8d\0"
"&lrm;\0"      "\xe2\x80\x8e\0"
"&rlm;\0"      "\xe2\x80\x8f\0"
;

static inline void
_escaped_advance_after_end_of_string(const char **p_buf)
{
   while (**p_buf != 0) (*p_buf)++;
   (*p_buf)++;
}

static inline int
_escaped_is_eq_and_advance(const char *s, const char *s_end,
      const char **p_m, const char *m_end)
{
   Eina_Bool reached_end;
   for (;((s < s_end) && (*p_m < m_end)); s++, (*p_m)++)
     {
        if (*s != **p_m)
          {
             _escaped_advance_after_end_of_string(p_m);
             return 0;
          }
     }

   reached_end = !**p_m;
   if (*p_m < m_end)
     _escaped_advance_after_end_of_string(p_m);

   return ((s == s_end) && reached_end);
}

static inline const char *
_escaped_char_get(const char *s, const char *s_end)
{
   /* Handle numeric escape codes. */
   if (s[1] == '#')
     {
        static char utf8_escape[7]; /* Support up to 6 bytes utf8 */
        char ustr[10];
        Eina_Unicode uchar[2] = { 0, 0 };
        char *utf8_char;
        size_t len = 0;
        int base = 10;
        s += 2; /* Skip "&#" */

        if ((*s == 'x') || (*s == 'X'))
          {
             s++;
             base = 16;
          }

        len = s_end - s;
        if (len > sizeof(ustr))
           len = sizeof(ustr);

        memcpy(ustr, s, len);
        ustr[len - 1] = '\0';
        uchar[0] = strtol(ustr, NULL, base);

        if (uchar[0] == 0)
          return NULL;

        utf8_char = eina_unicode_unicode_to_utf8(uchar, NULL);
        // eina_unicode_unicode_to_utf8() always creates a string that
        // is nul terminated - guaranteed
        if (utf8_char)
          {
             strcpy(utf8_escape, utf8_char);
             free(utf8_char);
          }

        return utf8_escape;
     }
   else
     {
        const char *map_itr, *map_end;

        map_itr = escape_strings;
        map_end = map_itr + sizeof(escape_strings);

        while (map_itr < map_end)
          {
             if (_escaped_is_eq_and_advance(s, s_end, &map_itr, map_end))
                return map_itr;
             if (map_itr < map_end)
                _escaped_advance_after_end_of_string(&map_itr);
          }
     }

   return NULL;
}

static char *
_text_util_markup_to_text(const char *text)
{
   Eina_Strbuf *sbuf;
   char *s, *p, *ret;
   char *tag_start, *tag_end, *esc_start, *esc_end;

   if (!text) return NULL;

   tag_start = tag_end = esc_start = esc_end = NULL;
   sbuf = eina_strbuf_new();
   p = (char *)text;
   s = p;
   /* This loop goes through all of the mark up text until it finds format
    * tags, escape sequences or the terminating NULL. When it finds either
    * of those, it appends the text found up until that point to the textblock
    * proccesses whatever found. It repeats itself until the termainating
    * NULL is reached. */
   for (;;)
     {
        /* If we got to the end of string or just finished/started tag
         * or escape sequence handling. */
        if ((*p == 0) ||
              (tag_end) || (esc_end) ||
              (tag_start) || (esc_start))
          {
             if (tag_end)
               {
                  /* If we reached to a tag ending, analyze the tag */
                  char *ttag;
                  size_t ttag_len;

                  tag_start++; /* Skip the < */
                  tag_end--; /* Skip the > */
                  if ((tag_end > tag_start) && (*(tag_end - 1) == '/'))
                    {
                       tag_end --; /* Skip the terminating '/' */
                       while (*(tag_end - 1) == ' ')
                         tag_end--; /* skip trailing ' ' */
                    }

                  ttag_len = tag_end - tag_start;

                  ttag = malloc(ttag_len + 1);
                  if (ttag)
                    {
                       memcpy(ttag, tag_start, ttag_len);
                       ttag[ttag_len] = 0;

                       if (_IS_PARAGRAPH_SEPARATOR_SIMPLE(ttag))
                          eina_strbuf_append(sbuf, _PARAGRAPH_SEPARATOR_UTF8);
                       else if (_IS_LINE_SEPARATOR(ttag))
                          eina_strbuf_append(sbuf, _NEWLINE_UTF8);
                       else if (_IS_TAB(ttag))
                          eina_strbuf_append(sbuf, _TAB_UTF8);
                       else if (!strncmp(ttag, "item", 4))
                          eina_strbuf_append(sbuf, _REPLACEMENT_CHAR_UTF8);

                       free(ttag);
                    }
                  tag_start = tag_end = NULL;
               }
             else if (esc_end)
               {
                  const char *escape;

                  escape = _escaped_char_get(esc_start, esc_end + 1);
                  if (escape) eina_strbuf_append(sbuf, escape);
                  esc_start = esc_end = NULL;
               }
             else if (*p == 0)
               {
                  if (s)
                    {
                       eina_strbuf_append_length(sbuf, s, p - s);
                       s = NULL;
                    }
                  else
                    {
                       ERR("There is a invalid markup tag at positoin '%u'. Please check the text.", (unsigned int) (p - text));
                    }
               }
             if (*p == 0)
                break;
          }
        if (*p == '<')
          {
             if (!esc_start)
               {
                  /* Append the text prior to this to the textblock and
                   * mark the start of the tag */
                  tag_start = p;
                  tag_end = NULL;
                  if (s)
                    {
                       eina_strbuf_append_length(sbuf, s, p - s);
                       s = NULL;
                    }
                  else
                    {
                       ERR("There is a invalid markup tag at positoin '%u'. Please check the text.", (unsigned int) (p - text));
                    }
               }
          }
        else if (*p == '>')
          {
             if (tag_start)
               {
                  tag_end = p + 1;
                  s = p + 1;
               }
          }
        else if (*p == '&')
          {
             if (!tag_start)
               {
                  /* Append the text prior to this to the textblock and mark
                   * the start of the escape sequence */
                  esc_start = p;
                  esc_end = NULL;
                  if (s)
                    {
                       eina_strbuf_append_length(sbuf, s, p - s);
                       s = NULL;
                    }
                  else
                    {
                       ERR("There is a invalid markup tag at positoin '%u'. Please check the text.", (unsigned int) (p - text));
                    }
               }
          }
        else if (*p == ';')
          {
             if (esc_start)
               {
                  esc_end = p;
                  s = p + 1;
               }
          }
        p++;
     }

   ret = eina_strbuf_string_steal(sbuf);
   eina_strbuf_free(sbuf);
   return ret;
}

static EOLIAN char*
_efl_text_markup_util_markup_to_text(const char *text)
{
   return _text_util_markup_to_text(text);
}

#include "interfaces/efl_text_markup_util.eo.c"
