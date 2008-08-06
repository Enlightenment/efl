#ifndef EINA_INLINE_HASH_X_
#define EINA_INLINE_HASH_X_

/*
  djb2 hash algorithm was first reported by dan bernstein, and was the old
  default hash function for evas.
 */
static inline int
eina_hash_djb2(const char *key, int len)
{
   unsigned int hash_num = 5381;
   const unsigned char *ptr;

   if (!key) return 0;

   for (ptr = (unsigned char *)key; len; ptr++, len--)
     hash_num = ((hash_num << 5) + hash_num) ^ *ptr; /* hash * 33 ^ c */

   hash_num &= 0xff;
   return (int)hash_num;
}

#endif
