#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>

#include "evil_private.h"


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
           return localeconv()->decimal_point;
        }
      case D_T_FMT:
        {
           return "%a %d %b %Y %T %Z";
        }
      case D_FMT:
        {
           return "%m/%d/%Y";
        }
      case T_FMT:
        {
           return "%T";
        }
      case T_FMT_AMPM:
        {
           return "%r";
        }
      default:
        {
           return "%a %d %b %Y %T %Z";
        }
     }

   return nothing;
}
