
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
