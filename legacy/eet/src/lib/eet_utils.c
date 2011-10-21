#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdio.h>
#include <math.h>

#include "Eet.h"
#include "Eet_private.h"

int
_eet_hash_gen(const char *key,
              int         hash_size)
{
   int hash_num = 0;
   int value, i;
   int mask;
   unsigned char *ptr;

   /* no string - index 0 */
   if (!key)
     return 0;

   /* calc hash num */
   for (i = 0, ptr = (unsigned char *)key, value = (int)(*ptr);
        value;
        ptr++, i++, value = (int)(*ptr))
     hash_num ^= (value | (value << 8)) >> (i & 0x7);

   /* mask it */
   mask = (1 << hash_size) - 1;
   hash_num &= mask;
   /* return it */
   return hash_num;
} /* _eet_hash_gen */

