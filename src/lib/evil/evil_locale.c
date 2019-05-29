#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <locale.h>
#include <errno.h>

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef EAPI
# undef EAPI
#endif

#ifdef EFL_BUILD
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif
#else
# define EAPI __declspec(dllimport)
#endif

#include "evil_locale.h" /* LC_MESSAGES */

/*
 * LOCALE_SISO639LANGNAME and LOCALE_SISO3166CTRYNAME need at least a buffer
 * of 9 char each (including NULL char). So we need 2*8 + the trailing NULL
 * char + '_', so 18 char.
 */
static char _evil_locale_buf[18];

char *evil_setlocale(int category, const char *locale)
{
   char buf[9];
   int l1;
   int l2;

   if (category != LC_MESSAGES)
     return setlocale(category, locale);

   if (locale != NULL)
     {
        errno = EINVAL;
        return NULL;
     }

   l1 = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME,
                      buf, sizeof(buf));
   if (!l1) return NULL;

   memcpy(_evil_locale_buf, buf, l1 - 1);
   _evil_locale_buf[l1 - 1] = '_';

   l2 = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME,
                      buf, sizeof(buf));
   if (!l2) return NULL;

   memcpy(_evil_locale_buf + l1, buf, l2);

   return _evil_locale_buf;
}
