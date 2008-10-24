/* ecore_value.c

Copyright (C) 2001 Christopher Rosendahl    <smugg@fatelabs.com>
                   Nathan Ingersoll         <ningerso@d.umn.edu>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "Ecore.h"
#include "Ecore_Data.h"
#include "ecore_private.h"

EAPI const unsigned int ecore_prime_table[] =
{
   17, 31, 61, 127, 257, 509, 1021,
     2053, 4093, 8191, 16381, 32771, 65537, 131071, 262147, 524287, 1048573,
     2097143, 4194301, 8388617, 16777213
};

EAPI inline void 
ecore_print_warning(const char *function, const char *sparam)
{
   fprintf(stderr, "***** Developer Warning ***** :\n"
	   "\tThis program is calling:\n\n"
	   "\t%s();\n\n"
	   "\tWith the parameter:\n\n"
	   "\t%s\n\n"
	   "\tbeing NULL. Please fix your program.\n", function, sparam);
   fflush(stderr);
   if (getenv("ECORE_ERROR_ABORT")) abort();
}

/**
 * Just casts the key to an unsigned int
 * @param  key The key to return compute a hash value
 * @return The key cast to an unsigned int.
 */
EAPI unsigned int 
ecore_direct_hash(const void *key)
{
#ifdef __LP64__
   unsigned long int val = (unsigned long int)key;

   return (unsigned int) ((val >> 32) ^ val);
#else
   return (unsigned int) key;
#endif
}

/**
 * Compute the hash value of a string
 * @param  key A pointer to the string to compute a hash value
 * @return A computed hash value for @a key.
 */
EAPI unsigned int 
ecore_str_hash(const void *key)
{
   int i;
   unsigned int mask;
   unsigned int value = 0;
   const char *k = key;

   if (!k)
     return 0;

   mask = (sizeof(unsigned int) * 8) - 1;

   for (i = 0; k[i] != '\0'; i++)
     {
	value ^= ((unsigned int) k[i] << ((i * 5) & mask));
     }

   return value;
}

/**
 * Perform a direct comparison of two keys' values
 * @param  key1 The first key to compare
 * @param  key2 The second key to compare
 * @return A strcmp style value to indicate the larger key
 */
EAPI int 
ecore_direct_compare(const void *key1, const void *key2)
{
   unsigned int k1, k2;

   k1 = (unsigned int) key1;
   k2 = (unsigned int) key2;

   if (k1 > k2)
     return 1;

   if (k1 < k2)
     return -1;

   return 0;
}

/**
 * Perform a string comparison of two keys values
 * @param  key1 The first key to compare
 * @param  key2 The second key to compare
 * @return A strcmp style value to indicate the larger key
 */
EAPI int 
ecore_str_compare(const void *key1, const void *key2)
{
   const char *k1, *k2;

   if (!key1 || !key2)
     return ecore_direct_compare(key1, key2);
   else if (key1 == key2)
     return 0;

   k1 = key1;
   k2 = key2;

   return strcmp(k1, k2);
}
