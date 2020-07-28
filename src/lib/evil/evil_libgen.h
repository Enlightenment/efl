#ifndef __EVIL_LIBGEN_H__
#define __EVIL_LIBGEN_H__

#include <evil_private.h>

#ifndef HAVE_BASENAME
# define HAVE_BASENAME
# define basename evil_basename
#endif

#ifndef HAVE_DIRNAME
# define HAVE_DIRNAME
# define dirname evil_dirname
#endif

EVIL_API char* evil_basename (char* path);

EVIL_API char* evil_dirname (char* path);

#endif

