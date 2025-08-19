#include "evas_engine.h"

Outbuf *
_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h)
{
   Outbuf *ob;

   ob = calloc(1, sizeof(Outbuf));
   if (!ob) return NULL;

   ob->w = w;
   ob->h = h;
   ob->info = info;

   return ob;
}

void
_outbuf_free(Outbuf *ob)
{
   free(ob);
}
