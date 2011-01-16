#include "evas_common.h"
#include "evas_encoding.h"

EAPI Eina_Unicode
evas_common_encoding_utf8_get_next(const char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the next code point after this.
    *
    * Returns 0 to indicate there is no next char
    */
#if 1
   int index = *iindex;
   Eina_Unicode r;
   unsigned char d;

   /* if this char is the null terminator, exit */
   if ((d = buf[index++]) == 0) return 0;
     
   if ((d & 0x80) == 0)
     { // 1 byte (7bit) - 0xxxxxxx
        *iindex = index;
        return d;
     }
   if ((d & 0xe0) == 0xc0)
     { // 2 byte (11bit) - 110xxxxx 10xxxxxx
        r  = (d & 0x1f) << 6;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f);
        if (!r) return 0;
        *iindex = index;
        return r;
     }
   if ((d & 0xf0) == 0xe0)
     { // 3 byte (16bit) - 1110xxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x0f) << 12;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 6;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f);
        if (!r) return 0;
        *iindex = index;
        return r;
     }
   if ((d & 0xf8) == 0xf0)
     { // 4 byte (21bit) - 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x07) << 18;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 12;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 6;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f);
        if (!r) return 0;
        *iindex = index;
        return r;
     }
   if ((d & 0xfc) == 0xf8)
     { // 5 byte (26bit) - 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x03) << 24;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 18;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 12;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 6;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f);
        if (!r) return 0;
        *iindex = index;
        return r;
     }
   if ((d & 0xfe) == 0xfc)
     { // 6 byte (31bit) - 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x01) << 30;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 24;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 18;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 12;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f) << 6;
        if ((d = buf[index++]) == 0) return 0;
        r |= (d & 0x3f);
        if (!r) return 0;
        *iindex = index;
        return r;
     }
   return 0;
#else
   int index = *iindex, r, len;
   unsigned char d, d2, d3, d4;
   
   /* if this char is the null terminator, exit */
   if (!buf[index]) return 0;
     
   d = buf[index++];

   while (buf[index] && ((buf[index] & 0xc0) == 0x80))
     index++;
   len = index - *iindex;

   if (len == 1)
      r = d;
   else if (len == 2)
     {
	/* 2 bytes */
        d2 = buf[*iindex + 1];
        if (d2 == 0)
          {
             *iindex = *iindex + 1;
             return 0;
          }
	r = d & 0x1f; /* copy lower 5 */
	r <<= 6;
	r |= (d2 & 0x3f); /* copy lower 6 */
     }
   else if (len == 3)
     {
	/* 3 bytes */
        d2 = buf[*iindex + 1];
        if (d2 == 0)
          {
             *iindex = *iindex + 1;
             return 0;
          }
        d3 = buf[*iindex + 2];
        if (d3 == 0)
          {
             *iindex = *iindex + 2;
             return 0;
          }
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
     }
   else
     {
	/* 4 bytes */
        d2 = buf[*iindex + 1];
        if (d2 == 0)
          {
             *iindex = *iindex + 1;
             return 0;
          }
        d3 = buf[*iindex + 2];
        if (d3 == 0)
          {
             *iindex = *iindex + 2;
             return 0;
          }
        d4 = buf[*iindex + 3];
        if (d4 == 0)
          {
             *iindex = *iindex + 3;
             return 0;
          }
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
	r <<= 6;
	r |= (d4 & 0x3f);
     }
   *iindex = index;
   return r;
#endif
}

EAPI Eina_Unicode
evas_common_encoding_utf8_get_prev(const char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the prev code point after this.
    *
    * Returns 0 to indicate there is no prev char
    */

   int r;
   int index = *iindex;
   /* although when index == 0 there's no previous char, we still want to get
    * the current char */
   if (index < 0) 
     return 0;

   /* First obtain the codepoint at iindex */
   r = evas_common_encoding_utf8_get_next(buf, &index);

   /* Next advance iindex to previous codepoint */
   index = *iindex;
   index--;
   while ((index > 0) && ((buf[index] & 0xc0) == 0x80))
     index--;

   *iindex = index;
   return r;
}

EAPI Eina_Unicode
evas_common_encoding_utf8_get_last(const char *buf, int buflen)
{
   /* jumps to the nul byte at the buffer end and decodes backwards and
    * returns the offset index byte in the buffer where the last character
    * in the buffer begins.
    *
    * Returns -1 to indicate an error
    */
   int index;
   unsigned char d;

   if (buflen < 1) return 0;
   index = buflen - 1;
   d = buf[index];
   if (!(d & 0x80))
     return index;
   else
     {
	while (index > 0)
	  {
	     index--;
	     d = buf[index];
	     if ((d & 0xc0) != 0x80)
	       return index;
	  }
     }
   return 0;
}

EAPI int
evas_common_encoding_utf8_get_len(const char *buf)
{
   /* returns the number of utf8 characters (not bytes) in the string */
   int index = 0, len = 0;

   while (buf[index])
     {
	if ((buf[index] & 0xc0) != 0x80)
	  len++;
	index++;
     }
   return len;
}

/* FIXME: Should optimize! */
EAPI Eina_Unicode *
evas_common_encoding_utf8_to_unicode(const char *utf, int *_len)
{
   int len, i;
   int index;
   Eina_Unicode *buf, *ind;

   len = evas_common_encoding_utf8_get_len(utf);
   if (_len)
      *_len = len;
   buf = (Eina_Unicode *) calloc(sizeof(Eina_Unicode), (len + 1));
   if (!buf) return buf;

   for (i = 0, index = 0, ind = buf ; i < len ; i++, ind++)
     {
        *ind = evas_common_encoding_utf8_get_next(utf, &index);
     }

   return buf;
}

EAPI char *
evas_common_encoding_unicode_to_utf8(const Eina_Unicode *uni, int *_len)
{
   char *buf;
   const Eina_Unicode *uind;
   char *ind;
   int ulen, len;

   ulen = eina_unicode_strlen(uni);
   buf = (char *) calloc(ulen + 1, EVAS_ENCODING_UTF8_BYTES_PER_CHAR);

   len = 0;
   for (uind = uni, ind = buf ; *uind ; uind++)
     {
        if (*uind <= 0x7F) /* 1 byte char */
          {
             *ind++ = *uind;
             len += 1;
          }
        else if (*uind <= 0x7FF) /* 2 byte char */
          {
             *ind++ = 0xC0 | (unsigned char) (*uind >> 6);
             *ind++ = 0x80 | (unsigned char) (*uind & 0x3F);
             len += 2;
          }
        else if (*uind <= 0xFFFF) /* 3 byte char */
          {
             *ind++ = 0xE0 | (unsigned char) (*uind >> 12);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 6) & 0x3F);
             *ind++ = 0x80 | (unsigned char) (*uind & 0x3F);
             len += 3;
          }
        else /* 4 byte char */
          {
             *ind++ = 0xF0 | (unsigned char) ((*uind >> 18) & 0x07);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 12) & 0x3F);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 6) & 0x3F);
             *ind++ = 0x80 | (unsigned char) (*uind & 0x3F);
             len += 4;
          }
     }
   buf = realloc(buf, len + 1);
   buf[len] = '\0';
   if (_len)
      *_len = len;
   return buf;
}


