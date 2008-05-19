/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <stdio.h>
#include <math.h>

#include "Eet.h"
#include "Eet_private.h"

int
_eet_hash_gen(const char *key, int hash_size)
{
   int			hash_num = 0;
   int			value, i;
   unsigned char	*ptr;

   const int masks[9] =
     {
	0x00,
	0x01,
	0x03,
	0x07,
	0x0f,
	0x1f,
	0x3f,
	0x7f,
	0xff
     };

   /* no string - index 0 */
   if (!key) return 0;

   /* calc hash num */
   for (i = 0, ptr = (unsigned char *)key, value = (int)(*ptr);
	value;
	ptr++, i++, value = (int)(*ptr))
     hash_num ^= (value | (value << 8)) >> (i & 0x7);

   /* mask it */
   hash_num &= masks[hash_size];
   /* return it */
   return hash_num;
}

/* On Windows (using MinGW or VC++), printf-like functions */
/* rely on MSVCRT, which does not fully support the C99    */
/* specifications. In particular, they do not support the  */
/* modifier character %a.                                  */


/* That function converts a string created by a valid %a   */
/* modifier to a double.                                   */
/*                                                         */
/* The string must have the following format:              */
/*                                                         */
/*  [-]0xh.hhhhhp[+-]e                                     */
/*                                                         */
/* where e is a decimal number.                            */
/* If n is the number of cyphers after the point, the      */
/* returned mantisse and exponents are                     */
/*                                                         */
/*  mantisse: [-]hhhhhh                                    */
/*  exponent: 2^([+-]e - 4 * n)                            */
int
_eet_string_to_double_convert(const char *src, long long *m, long *e)
{
   const char *str;
   long long   mantisse;
   long        exponent;
   int         nbr_decimals;
   int         sign;

   str = src;
   sign = +1;

   if (*str == '-')
     {
        sign = -1;
        str++;
     }
   if (*str == '0')
     {
        str++;
        if (*str == 'x')
          str++;
        else
          {
             fprintf(stderr, "[Eet] Error 1 during conversion of '%s'\n", src);
             return 0;
          }
     }
   else
     {
        fprintf(stderr, "[Eet] Error 2 during conversion of '%s'\n", src);
        return 0;
     }

   nbr_decimals = 0;
   mantisse = (*str >= 'a') ? *str - 'a'  + 10 : *str - '0';
   str++;
   if (*str == '.')
     {
        str++;
        while (*str != 'p')
          {
             mantisse <<= 4;
             mantisse += (*str >= 'a') ? *str - 'a'  + 10 : *str - '0';
             str++;
             nbr_decimals++;
          }
     }
   if (sign < 0)
     mantisse = -mantisse;
   if (*str != 'p')
     {
        fprintf(stderr, "[Eet] Error 3 during conversion '%s'\n", src);
        return 0;
     }
   sign = +1;
   str++;
   if (*str == '-')
     {
        sign = -1;
        str++;
     }
   else if (*str == '+') str++;

   exponent = 0;
   while (*str != '\0')
     {
        exponent *= 10;
        exponent += *str - '0';
        str++;
     }

   if (sign < 0)
     exponent = -exponent;

   *m = mantisse;
   *e = exponent - (nbr_decimals << 2);

   return 1;
}

/* That function converts a double to a string that as the */
/* following format:                                       */
/*                                                         */
/*  [-]0xh.hhhhhp[+-]e                                     */
/*                                                         */
/* where h is a hexadecimal number and e a decimal number. */
void
_eet_double_to_string_convert(char des[128], double d)
{
   static const char look_up_table[] = {'0', '1', '2', '3', '4',
                                        '5', '6', '7', '8', '9',
                                        'a', 'b', 'c', 'd', 'e', 'f'};
   int p;
   int i;

   if (d < 0.0)
     {
        *(des++) = '-';
        d = -d;
     }

   d = frexp(d, &p);

   if (p)
     {
        d *= 2;
        p -= 1;
     }

   *(des++) = '0';
   *(des++) = 'x';
   *(des++) = look_up_table[(size_t)d];
   *(des++) = '.';

   for (i = 0; i < 16; i++)
     {
        d -= floor(d);
        d *= 16;
        *(des++) = look_up_table[(size_t)d];
     }

   while (*(des - 1) == '0')
     des--;

   if (*(des - 1) == '.')
     des--;

   *(des++) = 'p';
   if (p < 0)
     {
        *(des++) = '-';
        p = -p;
     }
   else
     *(des++) = '+';

   snprintf(des, 128, "%d", p);
}
