#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"
#include "evil_private.h"

#ifndef localtime_r

struct tm *
localtime_r(const time_t *timep, struct tm *result)
{
# ifndef _MSC_VER
   struct tm *tmp;
# endif /* ! _MSC_VER */

   if (!timep || !result)
     return NULL;

# ifdef _MSC_VER
   if (localtime_s(result, timep) != 0)
     return NULL;
# else
   tmp = localtime(timep);
   if (!tmp)
     return NULL;

   memcpy(result, tmp, sizeof(struct tm));

# endif /* ! _MSC_VER */

   return result;
}

#endif /* localtime_r */
