#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "Ecore.h"
#include "ecore_private.h"

#include "Ecore_Input.h"
#include "ecore_input_private.h"

// some info on a big big big compose table
// http://cgit.freedesktop.org/xorg/lib/libX11/plain/nls/en_US.UTF-8/Compose.pre
// isolate compose tree into its own file - hand crafted into static const c
#include "ecore_input_compose.h"

EAPI Ecore_Compose_State
ecore_compose_get(const Eina_List *seq, char **seqstr_ret)
{
   Comp *c, *cend;
   Eina_List *l;
   const char *s;
   int i = 0;

   if (!seq) return ECORE_COMPOSE_NONE;
   l = (Eina_List *)seq;
   s = l->data;
   cend = (Comp *)comp + (sizeof(comp) / sizeof(comp[0]));
   for (c = (Comp *)comp; c->s && s;)
     {
        // doesn't match -> jump to next level entry
        if (!(!strcmp(s, c->s)))
          {
             c += c->jump + 1;
             if (c >= cend)
               {
                  return ECORE_COMPOSE_NONE;
               }
          }
        else
          {
             cend = c + c->jump;
             // advance to next sequence member
             l = l->next;
             i++;
             if (l) s = l->data;
             else s = NULL;
             c++;
             // if advanced item jump is an endpoint - it's the string we want
             if (c->jump == 0)
               {
                  if (seqstr_ret) *seqstr_ret = strdup(c->s);
                  return ECORE_COMPOSE_DONE;
               }
          }
     }
   if (i == 0) return ECORE_COMPOSE_NONE;
   return ECORE_COMPOSE_MIDDLE;
}
