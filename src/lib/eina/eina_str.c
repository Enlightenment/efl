/* Leave the OpenBSD version below so we can track upstream fixes */
/*      $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $        */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#ifdef HAVE_ICONV
# include <errno.h>
# include <iconv.h>
#endif

#include "eina_private.h"
#include "eina_str.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

/*
 * Internal helper function used by eina_str_has_suffix() and
 * eina_str_has_extension()
 */
static inline Eina_Bool
eina_str_has_suffix_helper(const char *str,
                           const char *suffix,
                           int (*cmp)(const char *, const char *))
{
   size_t str_len;
   size_t suffix_len;

   if ((!str) || (!suffix)) return EINA_FALSE;
   str_len = strlen(str);
   suffix_len = eina_strlen_bounded(suffix, str_len);
   if (suffix_len == (size_t)-1)
      return EINA_FALSE;

   return cmp(str + str_len - suffix_len, suffix) == 0;
}

static inline char **
eina_str_split_full_helper(const char *str,
                           const char *delim,
                           int max_tokens,
                           unsigned int *elements)
{
   char *s, *pos, **str_array;
   const char *src;
   size_t len, dlen;
   unsigned int tokens = 0, x;
   const char *idx[256] = {NULL};

   if ((!str) || (!delim))
     {
        if (elements)
          *elements = 0;

        return NULL;
     }
   if (max_tokens < 0) max_tokens = 0;
   if (max_tokens == 1)
     {
        str_array = malloc(sizeof(char *) * 2);
        if (!str_array)
          {
             if (elements)
                *elements = 0;

             return NULL;
          }

        s = strdup(str);
        if (!s)
          {
             free(str_array);
             if (elements)
                *elements = 0;

             return NULL;
          }
        if (elements)
          *elements = 1;
        str_array[0] = s;
        str_array[1] = NULL;
        return str_array;
     }
   dlen = strlen(delim);
   if (dlen == 0)
     {
        if (elements)
           *elements = 0;

        return NULL;
     }

   src = str;
   /* count tokens and check strlen(str) */
   while (*src != '\0')
     {
        const char *d = delim, *d_end = d + dlen;
        const char *tmp = src;
        for (; (d < d_end) && (*tmp != '\0'); d++, tmp++)
          {
             if (EINA_LIKELY(*d != *tmp))
                break;
          }
        if (EINA_UNLIKELY(d == d_end))
          {
             src = tmp;
             if (tokens < (sizeof(idx) / sizeof(idx[0])))
               {
                  idx[tokens] = tmp;
                  //printf("token %d='%s'\n", tokens + 1, idx[tokens]);
               }
             tokens++;
             if (tokens && (tokens == (unsigned int)max_tokens)) break;
          }
        else
           src++;
     }
   len = src - str + strlen(src);

   str_array = malloc(sizeof(char *) * (tokens + 2));
   if (!str_array)
     {
        if (elements)
           *elements = 0;

        return NULL;
     }

   if (!tokens)
     {
        s = strdup(str);
        if (!s)
          {
             free(str_array);
             if (elements)
                *elements = 0;

             return NULL;
          }
        str_array[0] = s;
        str_array[1] = NULL;
        if (elements)
          *elements = 1;
        return str_array;
     }

   s = malloc(len + 1);
   if (!s)
     {
        free(str_array);
        if (elements)
           *elements = 0;

        return NULL;
     }

   str_array[0] = s;

   if (len == tokens * dlen)
     {
        /* someone's having a laugh somewhere */
        memset(s, 0, len + 1);
        for (x = 1; x < tokens + 1; x++)
          str_array[x] = s + x;
        str_array[x] = NULL;
        if (elements)
          *elements = x;
        return str_array;
     }
   /* copy tokens and string */
   if (idx[0] - str - dlen > len)
     {
        /* FIXME: don't think this can happen but putting this here just in case */
        abort();
     }
   pos = s;
   for (x = 0; x < MIN(tokens, (sizeof(idx) / sizeof(idx[0]))); x++)
     {
        if (x + 1 < (sizeof(idx) / sizeof(idx[0])))
          {
             /* first one is special */
             if (!x)
               {
                  eina_strlcpy(pos, str, idx[x] - str - dlen + 1);
                  str_array[x] = pos;
                  //printf("str_array[%d] = '%s'\n", x, str_array[x]);
                  pos += idx[x] - str - dlen + 1;
                  if ((tokens == 1) && (idx[0]))
                    {
                       eina_strlcpy(pos, idx[x], len + 1 - (pos - s));
                       x++, tokens++;
                       str_array[x] = pos;
                    }
               }
             /* more tokens */
             else if (idx[x + 1])
               {
                  eina_strlcpy(pos, idx[x - 1], idx[x] - idx[x - 1] - dlen + 1);
                  str_array[x] = pos;
                  //printf("str_array[%d] = '%s'\n", x, str_array[x]);
                  pos += idx[x] - idx[x - 1] - dlen + 1;
               }
             /* last token */
             else
               {
                  if (max_tokens && ((unsigned int)max_tokens < tokens + 1))
                    eina_strlcpy(pos, idx[x - 1], len + 1 - (pos - s));
                  else
                    {
                       //printf("diff: %d\n", len + 1 - (pos - s));
                       eina_strlcpy(pos, idx[x - 1], idx[x] - idx[x - 1] - dlen + 1);
                       str_array[x] = pos;
                       //printf("str_array[%d] = '%s'\n", x, str_array[x]);
                       pos += idx[x] - idx[x - 1] - dlen + 1;
                       x++, tokens++;
                       eina_strlcpy(pos, idx[x - 1], len + 1 - (pos - s));
                    }
                  str_array[x] = pos;
                  //printf("str_array[%d] = '%s'\n", x, str_array[x]);
               }
          }
        /* no more tokens saved after this one */
        else
          {
             eina_strlcpy(pos, idx[x - 1], idx[x] - idx[x - 1] - dlen + 1);
             str_array[x] = pos;
             //printf("str_array[%d] = '%s'\n", x, str_array[x]);
             pos += idx[x] - idx[x - 1] - dlen + 1;
             src = idx[x];
             x++, tokens++;
             str_array[x] = s = pos;
             break;
          }
     }
   if ((x != tokens) && ((!max_tokens) || (x < tokens)))
     {
        while (*src != '\0')
          {
             const char *d = delim, *d_end = d + dlen;
             const char *tmp = src;
             for (; (d < d_end) && (*tmp != '\0'); d++, tmp++)
               {
                  if (EINA_LIKELY(*d != *tmp))
                     break;
               }
             if (((!max_tokens) || (((tokens == (unsigned int)max_tokens) || x < tokens - 2))) && (EINA_UNLIKELY(d == d_end)))
               {
                  src = tmp;
                  *s = '\0';
                  s++, x++;
                  //printf("str_array[%d] = '%s'\n", x, str_array[x - 1]);
                  str_array[x] = s;
               }
             else
               {
                  *s = *src;
                  s++, src++;
               }
          }
        *s = 0;
     }
   str_array[tokens] = NULL;
   if (elements)
     *elements = tokens;

   return str_array;
}

/**
 * @endcond
 */

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI size_t
eina_strlcpy(char *dst, const char *src, size_t siz)
{
#ifdef HAVE_STRLCPY
   return strlcpy(dst, src, siz);
#else
   char *d = dst;
   const char *s = src;
   size_t n = siz;

   /* Copy as many bytes as will fit */
   if (n != 0)
      while (--n != 0)
        {
           if ((*d++ = *s++) == '\0')
              break;
        }

   /* Not enough room in dst, add NUL and traverse rest of src */
   if (n == 0)
     {
        if (siz != 0)
           *d = '\0';  /* NUL-terminate dst */

        while (*s++)
           ;
     }

   return(s - src - 1); /* count does not include NUL */
#endif
}

EAPI size_t
eina_strlcat(char *dst, const char *src, size_t siz)
{
   char *d = dst;
   const char *s = src;
   size_t n = siz;
   size_t dlen;

   /* Find the end of dst and adjust bytes left but don't go past end */
   while (n-- != 0 && *d != '\0')
      d++;
   dlen = d - dst;
   n = siz - dlen;

   if (n == 0)
     return(dlen + (s ? strlen(s) : 0));

   if (s != NULL)
     {
        while (*s != '\0') {
           if (n != 1)
             {
                *d++ = *s;
                n--;
             }

           s++;
        }
     }
   *d = '\0';

   return(dlen + (s - src)); /* count does not include NUL */
}

EAPI Eina_Bool
eina_str_has_prefix(const char *str, const char *prefix)
{
   size_t str_len;
   size_t prefix_len;

   str_len = strlen(str);
   prefix_len = eina_strlen_bounded(prefix, str_len);
   if (prefix_len == (size_t)-1)
     return EINA_FALSE;

   return (strncmp(str, prefix, prefix_len) == 0);
}

EAPI Eina_Bool
eina_str_has_suffix(const char *str, const char *suffix)
{
   return eina_str_has_suffix_helper(str, suffix, strcmp);
}

EAPI Eina_Bool
eina_str_has_extension(const char *str, const char *ext)
{
   return eina_str_has_suffix_helper(str, ext, strcasecmp);
}

EAPI char **
eina_str_split_full(const char *str,
                    const char *delim,
                    int max_tokens,
                    unsigned int *elements)
{
   return eina_str_split_full_helper(str, delim, max_tokens, elements);
}


EAPI char **
eina_str_split(const char *str, const char *delim, int max_tokens)
{
   return eina_str_split_full_helper(str, delim, max_tokens, NULL);
}

EAPI size_t
eina_str_join_len(char *dst,
                  size_t size,
                  char sep,
                  const char *a,
                  size_t a_len,
                  const char *b,
                  size_t b_len)
{
   size_t ret = a_len + b_len + 1;
   size_t off;

   if (size < 1)
     return ret;

   if (size <= a_len)
     {
        memcpy(dst, a, size - 1);
        dst[size - 1] = '\0';
        return ret;
     }

   memcpy(dst, a, a_len);
   off = a_len;

   if (size <= off + 1)
     {
        dst[size - 1] = '\0';
        return ret;
     }

   dst[off] = sep;
   off++;

   if (size <= off + b_len + 1)
     {
        memcpy(dst + off, b, size - off - 1);
        dst[size - 1] = '\0';
        return ret;
     }

   memcpy(dst + off, b, b_len);
   dst[off + b_len] = '\0';
   return ret;
}

#ifdef HAVE_ICONV
EAPI char *
eina_str_convert(const char *enc_from, const char *enc_to, const char *text)
{
   iconv_t ic;
   char *new_txt, *outp;
   const char *inp;
   size_t inb, outb, outlen, tob, outalloc;

   if (!text)
      return NULL;

   ic = iconv_open(enc_to, enc_from);
   if (ic == (iconv_t)(-1))
      return NULL;

   new_txt = malloc(64);
   inb = strlen(text);
   outb = 64;
   inp = text;
   outp = new_txt;
   outalloc = 64;
   outlen = 0;

   for (;; )
     {
        size_t count;

        tob = outb;
#ifdef __FreeBSD__
        count = iconv(ic, &inp, &inb, &outp, &outb);
#else
        count = iconv(ic, (char **)&inp, &inb, &outp, &outb);
#endif
        outlen += tob - outb;
        if (count == (size_t)(-1))
          {
             if (errno == E2BIG)
               {
                  new_txt = realloc(new_txt, outalloc + 64);
                  outp = new_txt + outlen;
                  outalloc += 64;
                  outb += 64;
               }
             else
               {
                  if (new_txt)
                     free(new_txt);

                  new_txt = NULL;
                  break;
               }
          }

        if (inb == 0)
          {
             if (outalloc == outlen)
                new_txt = realloc(new_txt, outalloc + 1);

             new_txt[outlen] = 0;
             break;
          }
     }
   iconv_close(ic);
   return new_txt;
}
#else
EAPI char *
eina_str_convert(const char *enc_from EINA_UNUSED,
                 const char *enc_to EINA_UNUSED,
                 const char *text EINA_UNUSED)
{
   return NULL;
}
#endif

#ifdef HAVE_ICONV
EAPI char *
eina_str_convert_len(const char *enc_from, const char *enc_to, const char *text, size_t len, size_t *retlen)
{
   iconv_t ic;
   char *new_txt, *outp;
   const char *inp;
   size_t inb, outb, outlen, tob, outalloc;

   if (retlen) *retlen = 0;
   if (!text) return NULL;

   ic = iconv_open(enc_to, enc_from);
   if (ic == (iconv_t)(-1))
      return NULL;

   new_txt = malloc(64);
   inb = len;
   outb = 64;
   inp = text;
   outp = new_txt;
   outalloc = 64;
   outlen = 0;

   for (;; )
     {
        size_t count;

        tob = outb;
#ifdef __FreeBSD__
        count = iconv(ic, &inp, &inb, &outp, &outb);
#else
        count = iconv(ic, (char **)&inp, &inb, &outp, &outb);
#endif
        outlen += tob - outb;
        if (count == (size_t)(-1))
          {
             if (errno == E2BIG)
               {
                  new_txt = realloc(new_txt, outalloc + 64);
                  outp = new_txt + outlen;
                  outalloc += 64;
                  outb += 64;
               }
             else
               {
                  if (new_txt)
                     free(new_txt);

                  new_txt = NULL;
                  break;
               }
          }

        if (inb == 0)
          {
             if (outalloc == outlen)
                new_txt = realloc(new_txt, outalloc + 1);

             new_txt[outlen] = 0;
             break;
          }
     }
   iconv_close(ic);
   if (retlen) *retlen = outlen;
   return new_txt;
}
#else
EAPI char *
eina_str_convert_len(const char *enc_from EINA_UNUSED, const char *enc_to EINA_UNUSED, const char *text EINA_UNUSED, size_t len EINA_UNUSED, size_t *retlen)
{
   if (retlen) *retlen = 0;
   return NULL;
}
#endif

EAPI char *
eina_str_escape(const char *str)
{
   char *s2, *d;
   const char *s;

   if (!str)
      return NULL;

   s2 = malloc((strlen(str) * 2) + 1);
   if (!s2)
      return NULL;

   for (s = str, d = s2; *s != 0; s++, d++)
     {
        if ((*s == ' ') || (*s == '\\') || (*s == '\''))
          {
             *d = '\\';
             d++;
          }

        *d = *s;
     }
   *d = 0;
   return s2;
}

EAPI void
eina_str_tolower(char **str)
{
   char *p;
   if ((!str) || (!(*str)))
      return;

   for (p = *str; (*p); p++)
      *p = tolower((unsigned char )(*p));
}

EAPI void
eina_str_toupper(char **str)
{
   char *p;
   if ((!str) || (!(*str)))
      return;

   for (p = *str; (*p); p++)
      *p = toupper((unsigned char)(*p));
}
