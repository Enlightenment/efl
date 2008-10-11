
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"


#ifdef __MINGW32CE__

/*
 * Error related functions
 *
 */

void perror (const char *s)
{
   fprintf(stderr, "[Windows CE] error\n");
}

/*
 * Stream related functions
 *
 */

void rewind(FILE *stream)
{
  fseek(stream, 0, SEEK_SET);
}

#endif /* __MINGW32CE__ */
