#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"


#ifndef __MINGW32CE__

static char *
replace(char *prev, char *value)
{
   if (!value)
     return prev;

   if (prev)
     free (prev);
   return strdup (value);
}

char *
nl_langinfo(nl_item index)
{
   static char *result = NULL;
   static char *nothing = "";

   switch (index)
     {
      case CODESET:
        {
           char *p;
           result = replace(result, setlocale(LC_CTYPE, NULL));
           if (!(p = strrchr(result, '.')))
             return nothing;

           if ((++p - result) > 2)
             strcpy(result, "cp");
           else
             *result = '\0';
           strcat(result, p);

           return result;
        }
      case RADIXCHAR:
        {
           /* FIXME: better returned value... */
           return ".";
        }
     }

   return nothing;
}

#endif /* ! __MINGW32CE__ */
