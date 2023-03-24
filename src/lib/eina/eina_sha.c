/* EINA - EFL data type library
 * Copyright (C) 2023 Carsten Haitzler
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


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "eina_private.h"
#include "eina_sha.h"
#include "eina_cpu.h"

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


EINA_API void
eina_sha1(const unsigned char *data, int size, unsigned char dst[20])
{
#define SHSH(n, v) ((((v) << (n)) & 0xffffffff) | ((v) >> (32 - (n))))
   unsigned int digest[5], word[80], wa, wb, wc, wd, we, t;
   unsigned char buf[64];
   const unsigned char *d;
   int idx, left, i;
   const unsigned int magic[4] =
   {
      0x5a827999,
      0x6ed9eba1,
      0x8f1bbcdc,
      0xca62c1d6
   };

   idx = 0;
   digest[0] = 0x67452301;
   digest[1] = 0xefcdab89;
   digest[2] = 0x98badcfe;
   digest[3] = 0x10325476;
   digest[4] = 0xc3d2e1f0;

   memset(buf, 0, sizeof(buf));
   for (left = size, d = data; left > 0; left--, d++)
     {
        if ((idx == 0) && (left < 64))
          {
             memset(buf, 0, 60);
             buf[60] = (size >> 24) & 0xff;
             buf[61] = (size >> 16) & 0xff;
             buf[62] = (size >> 8) & 0xff;
             buf[63] = (size) & 0xff;
          }
        buf[idx] = *d;
        idx++;
        if ((idx == 64) || (left == 1))
          {
             if ((left == 1) && (idx < 64)) buf[idx] = 0x80;
             for (i = 0; i < 16; i++)
               {
                  word[i]  = (unsigned int)buf[(i * 4)    ] << 24;
                  word[i] |= (unsigned int)buf[(i * 4) + 1] << 16;
                  word[i] |= (unsigned int)buf[(i * 4) + 2] << 8;
                  word[i] |= (unsigned int)buf[(i * 4) + 3];
               }
             for (i = 16; i < 80; i++)
               word[i] = SHSH(1,
                              word[i - 3 ] ^ word[i - 8 ] ^
                              word[i - 14] ^ word[i - 16]);
             wa = digest[0];
             wb = digest[1];
             wc = digest[2];
             wd = digest[3];
             we = digest[4];
             for (i = 0; i < 80; i++)
               {
                  if (i < 20)
                    t = SHSH(5, wa) + ((wb & wc) | ((~wb) & wd)) + we + word[i] + magic[0];
                  else if (i < 40)
                    t = SHSH(5, wa) + (wb ^ wc ^ wd) + we + word[i] + magic[1];
                  else if (i < 60)
                    t = SHSH(5, wa) + ((wb & wc) | (wb & wd) | (wc & wd)) + we + word[i] + magic[2];
                  else if (i < 80)
                    t = SHSH(5, wa) + (wb ^ wc ^ wd) + we + word[i] + magic[3];
                  we = wd;
                  wd = wc;
                  wc = SHSH(30, wb);
                  wb = wa;
                  wa = t;
               }
             digest[0] += wa;
             digest[1] += wb;
             digest[2] += wc;
             digest[3] += wd;
             digest[4] += we;
             idx = 0;
          }
     }

   t = eina_htonl(digest[0]); digest[0] = t;
   t = eina_htonl(digest[1]); digest[1] = t;
   t = eina_htonl(digest[2]); digest[2] = t;
   t = eina_htonl(digest[3]); digest[3] = t;
   t = eina_htonl(digest[4]); digest[4] = t;

   memcpy(dst, digest, 5 * 4);
}
