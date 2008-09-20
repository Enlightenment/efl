
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"


#if ! (defined(__CEGCC__) || defined(__MINGW32CE__))

static char *
replace(char *prev, char *value)
{
   if (value == NULL)
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
           if ((p = strrchr(result, '.' )) == NULL)
             return nothing;

           if ((++p - result) > 2)
             strcpy(result, "cp");
           else
             *result = '\0';
           strcat(result, p);

           return result;
        }
     }

   return nothing;
}

#endif /* __CEGCC__ || __MINGW32CE__ */
