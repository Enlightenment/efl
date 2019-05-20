/* EINA - EFL data type library
 * Copyright (C) 2016 ProFUSION embedded systems
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EINA_INLINE_SLICE_H
#define _EINA_INLINE_SLICE_H

#include <string.h>

static inline Eina_Slice
eina_rw_slice_slice_get(const Eina_Rw_Slice rw_slice)
{
   Eina_Slice ret;
   ret.len = rw_slice.len;
   ret.mem = rw_slice.mem;
   return ret;
}

static inline Eina_Rw_Slice
eina_slice_dup(const Eina_Slice slice)
{
   Eina_Rw_Slice ret;

   ret.len = slice.len;
   ret.mem = NULL;
   if (ret.len == 0) return ret;

   ret.mem = malloc(ret.len);
   if (ret.mem) memcpy(ret.mem, slice.mem, ret.len);
   else ret.len = 0;

   return ret;
}

static inline Eina_Rw_Slice
eina_rw_slice_dup(const Eina_Rw_Slice rw_slice)
{
   return eina_slice_dup(eina_rw_slice_slice_get(rw_slice));
}

static inline int
eina_slice_compare(const Eina_Slice a, const Eina_Slice b)
{
   const size_t len = a.len <= b.len ? a.len : b.len;
   if (len > 0)
     {
        int r = memcmp(a.mem, b.mem, len);
        if (r != 0) return r;
     }
   if (a.len < b.len) return -1;
   else if (a.len > b.len) return 1;
   else return 0;
}

static inline int
eina_rw_slice_compare(const Eina_Rw_Slice a, const Eina_Rw_Slice b)
{
   return eina_slice_compare(eina_rw_slice_slice_get(a),
                             eina_rw_slice_slice_get(b));
}

static inline Eina_Rw_Slice
eina_rw_slice_copy(const Eina_Rw_Slice dst, const Eina_Slice src)
{
   const size_t len = src.len <= dst.len ? src.len : dst.len;
   Eina_Rw_Slice ret;

   ret.len = len;
   ret.mem = dst.mem;
   if (len > 0) memcpy(ret.mem, src.mem, len);
   return ret;
}

static inline Eina_Slice
eina_slice_seek(const Eina_Slice slice, ssize_t offset, int whence)
{
   Eina_Slice ret;

   ret.len = 0;
   ret.mem = slice.mem;

   if (whence == SEEK_END)
     {
        whence = SEEK_SET;
        offset += slice.len;
     }

   if (whence == SEEK_SET)
     {
        if ((size_t)offset > slice.len) offset = slice.len;
        else if (offset < 0) offset = 0;
        ret.len = slice.len - offset;
        ret.mem = (const void *)(slice.bytes + offset);
     }
   return ret;
}

static inline Eina_Rw_Slice
eina_rw_slice_seek(const Eina_Rw_Slice rw_slice, ssize_t offset, int whence)
{
   Eina_Rw_Slice ret;

   ret.len = 0;
   ret.mem = rw_slice.mem;

   if (whence == SEEK_END)
     {
        whence = SEEK_SET;
        offset += rw_slice.len;
     }

   if (whence == SEEK_SET)
     {
        if ((size_t)offset > rw_slice.len) offset = rw_slice.len;
        else if (offset < 0) offset = 0;
        ret.len = rw_slice.len - offset;
        ret.mem = (void *)(rw_slice.bytes + offset);
     }
   return ret;
}

static inline const void *
eina_slice_strchr(const Eina_Slice slice, int c)
{
   if (slice.len != 0) return memchr(slice.mem, c, slice.len);
   return NULL;
}


static inline const void *
eina_slice_find(const Eina_Slice slice, const Eina_Slice needle)
{
   Eina_Slice s, n;
   uint8_t c;

   if ((slice.len != 0) && (needle.len != 0) && (slice.len >= needle.len))
     {
        if (needle.len == 1) return eina_slice_strchr(slice, needle.bytes[0]);
        if ((slice.len == needle.len) &&
            (memcmp(slice.mem, needle.mem, needle.len) == 0))
          return slice.mem;

        s.mem = slice.mem;
        s.len = slice.len - (needle.len - 1);

        c = needle.bytes[0];
        n.mem = (const void *)(needle.bytes + 1);
        n.len = needle.len - 1;

        while (s.len > 0)
          {
             const uint8_t *p = (const uint8_t *)eina_slice_strchr(s, c);
             size_t offset;

             if (p)
               {
                  p++;
                  if (memcmp(p, n.mem, n.len) == 0)
                    return (const void *)(p - 1);

                  offset = p - s.bytes;
                  s.bytes += offset;
                  s.len -= offset;
                  continue;
               }
             break;
          }
     }
   return NULL;
}

static inline Eina_Bool
eina_slice_startswith(const Eina_Slice slice, const Eina_Slice prefix)
{
   if ((prefix.len != 0) && (slice.len >= prefix.len))
     return memcmp(slice.mem, prefix.mem, prefix.len) == 0;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_slice_endswith(const Eina_Slice slice, const Eina_Slice suffix)
{
   if ((suffix.len != 0) && (slice.len >= suffix.len))
     return memcmp(slice.bytes + slice.len - suffix.len,
                   suffix.mem, suffix.len) == 0;
   return EINA_FALSE;
}

static inline void *
eina_rw_slice_strchr(const Eina_Rw_Slice rw_slice, int c)
{
   if (rw_slice.len != 0) return memchr(rw_slice.mem, c, rw_slice.len);
   return NULL;
}

static inline void *
eina_rw_slice_find(const Eina_Rw_Slice rw_slice, const Eina_Slice needle)
{
   return (void *)eina_slice_find(eina_rw_slice_slice_get(rw_slice), needle);
}

static inline Eina_Bool
eina_rw_slice_startswith(const Eina_Rw_Slice rw_slice, const Eina_Slice prefix)
{
   if ((prefix.len != 0) && (rw_slice.len >= prefix.len))
     return memcmp(rw_slice.mem, prefix.mem, prefix.len) == 0;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_rw_slice_endswith(const Eina_Rw_Slice rw_slice, const Eina_Slice suffix)
{
   if ((suffix.len != 0) && (rw_slice.len >= suffix.len))
     return memcmp(rw_slice.bytes + rw_slice.len - suffix.len,
                   suffix.mem, suffix.len) == 0;
   return EINA_FALSE;
}

static inline const void *
eina_slice_end_get(const Eina_Slice slice)
{
   return (const void *)(slice.bytes + slice.len);
}

static inline void *
eina_rw_slice_end_get(const Eina_Rw_Slice rw_slice)
{
   return (void *)(rw_slice.bytes + rw_slice.len);
}

static inline char *
eina_slice_strdup(const Eina_Slice slice)
{
   if (slice.len != 0)
     return eina_strndup((const char *)slice.mem, slice.len);
   return strdup("");
}

static inline char *
eina_rw_slice_strdup(const Eina_Rw_Slice rw_slice)
{
   if (rw_slice.len != 0)
     return eina_strndup((const char *)rw_slice.mem, rw_slice.len);
   return strdup("");
}

#endif /* _EINA_INLINE_SLICE_H */
