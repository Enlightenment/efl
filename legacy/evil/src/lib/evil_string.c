
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"


#ifdef __MINGW32CE__

/*
 * Error related functions
 *
 */

char *strerror (int errnum)
{
   return "[Windows CE] error\n";
}

#endif /* __MINGW32CE__ */


#ifndef __CEGCC__

/*
 * bit related functions
 *
 */

int ffs(int i)
{
   int size;
   int x;

   if (!i) return 1;

   /* remove the sign bit */
   x = i & -i;
   size = sizeof(int) << 3;
   for (i = size; i > 0; --i, x <<= 1)
     if (x & (1 << (size - 1))) return i;

   return x;
}

#endif /* ! __CEGCC__ */
