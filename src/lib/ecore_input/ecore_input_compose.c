#include "ecore_input_private.h"

// some info on a big big big compose table
// http://cgit.freedesktop.org/xorg/lib/libX11/plain/nls/en_US.UTF-8/Compose.pre
// isolate compose tree into its own file - hand crafted into static const c
#include "ecore_input_compose.h"

EAPI Ecore_Compose_State 
ecore_input_compose_get(const Eina_List *seq, char **seqstr_ret)
{
   const char *p, *pend;
   const unsigned char *psz;
   Eina_List *l;
   const char *s;
   int i = 0;
   static int complen = 0;

   if (!seq) return ECORE_COMPOSE_NONE;
   l = (Eina_List *)seq;
   s = l->data;

   // calc comp string len first time around
   if (complen == 0)
     {
        int zeros = 0;

        for (p = comp; ; p++)
          {
             if (!(*p)) zeros++;
             else zeros = 0;
             // end marker - 4 0 bytes in a row
             if (zeros == 4)
               {
                  complen = p - comp - 3;
                  break;
               }
          }
     }
   // walk special comp string/byte array looking for our match
   pend = comp + complen;
   for (p = comp; (p < pend) && s;)
     {
        int len, jump = -1, bsize = -1;
        
        len = strlen(p);
        psz = (unsigned char *)(p + len + 1);
        // decode jump amount to next entry
        if (!(psz[0] & 0x80)) // < 0x80
          {
             jump = psz[0];
             bsize = 1;
          }
        else if ((psz[0] & 0xc0) == 0xc0) // < 0x200000
          {
             jump = (((psz[0] & 0x1f) << 16) | (psz[1] << 8) | (psz[2]));
             bsize = 3;
          }
        else // >= 0x4000
          {
             jump = (((psz[0] & 0x3f) << 8) | (psz[1]));
             bsize = 2;
          }

        // doesn't match -> jump to next level entry
        if (!(!strcmp(s, p)))
          {
             p = p + jump;
             if (p >= pend) return ECORE_COMPOSE_NONE;
          }
        // matches
        else
          {
             pend = p + jump;
             // advance to next sequence member
             l = l->next;
             i++;
             if (l) s = l->data;
             else s = NULL;
             p = p + len + 1 + bsize;
             len = strlen(p);
             psz = (unsigned char *)(p + len + 1);
             // leaf nodes all are short so psz[0] has the full value
             if ((len + 2) == psz[0])
               {
                  // final leaf node, so return string here
                  if (seqstr_ret) *seqstr_ret = strdup(p);
                  return ECORE_COMPOSE_DONE;
               }
          }
     }
   if (i == 0) return ECORE_COMPOSE_NONE;
   return ECORE_COMPOSE_MIDDLE;
}

/* deprecated functions */
EINA_DEPRECATED EAPI Ecore_Compose_State
ecore_compose_get(const Eina_List *seq, char **seqstr_ret)
{
   return ecore_input_compose_get(seq, seqstr_ret);
}
