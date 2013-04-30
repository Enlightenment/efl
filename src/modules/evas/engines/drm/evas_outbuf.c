#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"

Outbuf *
evas_outbuf_setup(int w, int h, unsigned int rotation, unsigned int depth, Eina_Bool alpha)
{
   Outbuf *ob;

   /* try to allocate space for out outbuf structure */
   if (!(ob = calloc(1, sizeof(Outbuf))))
     return NULL;

   /* set some default outbuf properties */
   ob->w = w;
   ob->h = h;
   ob->rotation = rotation;
   ob->depth = depth;

   /* return the allocated outbuf structure */
   return ob;
}

void 
evas_outbuf_free(Outbuf *ob)
{
   /* free the allocated outbuf structure */
   free(ob);
}
